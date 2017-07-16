#include <boost/test/unit_test.hpp>

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/follower.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

class FollowerStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        getEntries.push_back(value);
    }

    std::vector<std::string> getEntries;
};

struct FollowerFixture;

namespace cr
{
    namespace raft
    {
        template <>
        struct DebugVisitor<FollowerFixture>
        {
            DebugVisitor()
            {
                storage = std::make_shared<cr::raft::MemStorage>();
                engine = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&FollowerStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setHeartbeatTimeout(50)
                    .setElectionTimeout(100, 200)
                    .setRandomSeed(0)
                    .build();
                engine->initialize(0);
            }

            ~DebugVisitor()
            {}

            void setVoteFor(boost::optional<std::uint64_t> voteFor)
            {
                engine->setVotedFor(voteFor);
            }

            void setLeaderId(boost::optional<std::uint64_t> leaderId)
            {
                engine->setLeaderId(leaderId);
            }

            void setCommitIndex(std::uint64_t commitIndex)
            {
                engine->setCommitIndex(commitIndex);
            }

            void setCurrentTerm(std::uint64_t currentTerm)
            {
                engine->setCurrentTerm(currentTerm);
            }

            auto makeEntry(std::uint64_t term, std::string value)
            {
                pb::Entry entry;
                entry.set_term(term);
                entry.set_value(value);
                return entry;
            }

            auto makeRequestVoteReqMsg(std::uint64_t fromNodeId, std::uint64_t destNodeId,
                std::uint64_t lastLogIndex, std::uint64_t lastLogTerm, std::uint64_t candidateTerm)
            {
                auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                raftMsg->set_dest_node_id(destNodeId);
                raftMsg->set_from_node_id(fromNodeId);
                raftMsg->set_msg_type(cr::raft::pb::RaftMsg::REQUEST_VOTE_REQ);

                auto& request = *(raftMsg->mutable_request_vote_req());
                request.set_last_log_index(lastLogIndex);
                request.set_last_log_term(lastLogTerm);
                request.set_candidate_term(candidateTerm);

                return raftMsg;
            }

            auto makeAppendEntriesReqMsg(std::uint64_t fromNodeId, std::uint64_t destNodeId,
                std::uint64_t leaderTerm, std::uint64_t leaderCommit,
                std::uint64_t prevLogIndex, std::uint64_t prevLogTerm,
                const std::vector<std::string>& entries = std::vector<std::string>())
            {
                auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                raftMsg->set_dest_node_id(destNodeId);
                raftMsg->set_from_node_id(fromNodeId);
                raftMsg->set_msg_type(cr::raft::pb::RaftMsg::APPEND_ENTRIES_REQ);

                auto& request = *(raftMsg->mutable_append_entries_req());
                request.set_leader_term(leaderTerm);
                request.set_leader_commit(leaderCommit);
                request.set_prev_log_index(prevLogIndex);
                request.set_prev_log_term(prevLogTerm);

                for (auto&& entry : entries)
                {
                    *request.add_entries() = makeEntry(prevLogTerm, entry);
                }

                return raftMsg;
            }

            // 校验日志复制成功
            int checkVoteSuccess(std::uint64_t candidateId, const pb::RequestVoteReq& request)
            {
                if (messages.size() != 1)
                {
                    return 1;
                }
                if (messages[0]->msg_type() != pb::RaftMsg::REQUEST_VOTE_RESP || !messages[0]->has_request_vote_resp())
                {
                    return 2;
                }
                auto& response = messages[0]->request_vote_resp();
                if (response.follower_term() != engine->getCurrentTerm())
                {
                    return 3;
                }
                if (response.follower_term() != request.candidate_term())
                {
                    return 4;
                }
                if (!engine->getVotedFor() || *engine->getVotedFor() != candidateId)
                {
                    return 5;
                }
                if (!response.success())
                {
                    return 6;
                }
                return 0;
            }

            int checkLogAppendSuccess(std::uint64_t leaderId, const pb::AppendEntriesReq& request)
            {
                if (messages.size() != 1)
                {
                    return 1;
                }
                if (!engine->getLeaderId() || *engine->getLeaderId() != leaderId)
                {
                    return 2;
                }
                if (messages[0]->msg_type() != pb::RaftMsg::APPEND_ENTRIES_RESP || !messages[0]->has_append_entries_resp())
                {
                    return 3;
                }
                auto& response = messages[0]->append_entries_resp();
                if (response.follower_term() != request.leader_term())
                {
                    return 4;
                }
                if (response.follower_term() != engine->getCurrentTerm())
                {
                    return 5;
                }
                if (response.last_log_index() != request.prev_log_index() + request.entries_size())
                {
                    return 6;
                }
                if (!response.success())
                {
                    return 7;
                }
                return 0;
            }

            cr::raft::RaftEngine::Builder builder;
            std::vector<cr::raft::RaftEngine::RaftMsgPtr> messages;
            std::shared_ptr<cr::raft::MemStorage> storage;
            FollowerStatMachine stateMachine;
            std::shared_ptr<cr::raft::RaftEngine> engine;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftFollower)

BOOST_FIXTURE_TEST_CASE(electionTimeout, cr::raft::DebugVisitor<FollowerFixture>)
{
    std::uint64_t nowTime = 0;
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = builder.getMinElectionTimeout() - 1;
    BOOST_CHECK_LE(engine->update(nowTime, messages), builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);
}

BOOST_FIXTURE_TEST_CASE(nextElectionTimeout, cr::raft::DebugVisitor<FollowerFixture>)
{
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 1, 0);

    std::uint64_t nowTime = 1;
    engine->pushMessageQueue(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getMinElectionTimeout() - 1;
    engine->pushMessageQueue(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getMinElectionTimeout() - 1;
    engine->pushMessageQueue(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getMinElectionTimeout() - 1;
    engine->pushMessageQueue(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);
}

// 任期为0，投票肯定失败
BOOST_FIXTURE_TEST_CASE(voteZeroTerm, cr::raft::DebugVisitor<FollowerFixture>)
{
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 0);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();
}

// 任期为1，投票成功
BOOST_FIXTURE_TEST_CASE(voteOneTerm, cr::raft::DebugVisitor<FollowerFixture>)
{
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 1);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);


    BOOST_CHECK_EQUAL(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();
}

// 任期为1，重复投票成功
BOOST_FIXTURE_TEST_CASE(voteRepeatedOneTerm, cr::raft::DebugVisitor<FollowerFixture>)
{
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 1);
    
    setVoteFor(2);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();
}

// 日志不匹配，投票失败
BOOST_FIXTURE_TEST_CASE(voteLogMismatch, cr::raft::DebugVisitor<FollowerFixture>)
{
    storage->append(1, { makeEntry(1, "1") });
    storage->append(2, { makeEntry(3, "2" ) });

    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 1);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();

    raftMsg = makeRequestVoteReqMsg(2, 1, 2, 2, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();

    raftMsg = makeRequestVoteReqMsg(2, 1, 1, 3, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();
}

// 日志匹配，投票成功
BOOST_FIXTURE_TEST_CASE(voteLogMatch, cr::raft::DebugVisitor<FollowerFixture>)
{
    storage->append(1, { makeEntry(1, "1") });
    storage->append(2, { makeEntry(3, "2") });

    auto raftMsg = makeRequestVoteReqMsg(2, 1, 2, 3, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();

    raftMsg = makeRequestVoteReqMsg(2, 1, 3, 3, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();

    raftMsg = makeRequestVoteReqMsg(2, 1, 0, 4, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, raftMsg->request_vote_req()), 0);
    messages.clear();
}

// 任期为1，空日志，成功
BOOST_FIXTURE_TEST_CASE(logAppendOneTerm, cr::raft::DebugVisitor<FollowerFixture>)
{
    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 0, 0);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    messages.clear();
}

// 任期为2，空日志，成功
BOOST_FIXTURE_TEST_CASE(logAppendTwoTerm, cr::raft::DebugVisitor<FollowerFixture>)
{
    setLeaderId(3);
    setCurrentTerm(1);

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 2, 0, 0, 0);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    messages.clear();
}

// 任期比当前小，空日志，失败
BOOST_FIXTURE_TEST_CASE(logAppendTermLittle, cr::raft::DebugVisitor<FollowerFixture>)
{
    setLeaderId(3);
    setCurrentTerm(2);

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 0, 0);

    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);

    BOOST_CHECK_NE(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->append_entries_resp().follower_term(), 2);
    messages.clear();
}

// 日志不匹配
BOOST_FIXTURE_TEST_CASE(logAppendLogMismatch, cr::raft::DebugVisitor<FollowerFixture>)
{
    storage->append(1, { makeEntry(1,"1") });
    storage->append(2, { makeEntry(2,"2") });

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 3, 2);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->append_entries_resp().last_log_index(), 2);
    messages.clear();

    raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 2, 1);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_NE(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->append_entries_resp().last_log_index(), 1);
    BOOST_CHECK_EQUAL(storage->getLastIndex(), 1);
    messages.clear();
}

// 日志不匹配
BOOST_FIXTURE_TEST_CASE(logAppendLogMatch, cr::raft::DebugVisitor<FollowerFixture>)
{
    storage->append(1, { makeEntry(1,"1") });
    storage->append(2, { makeEntry(2,"2") });

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 2, 2);
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    messages.clear();
}

// 追加3条日志
BOOST_FIXTURE_TEST_CASE(logAppendLogThreeEntry, cr::raft::DebugVisitor<FollowerFixture>)
{
    storage->append(1, { makeEntry(1,"1") });
    storage->append(2, { makeEntry(2,"2") });
    setCurrentTerm(2);

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 2, 4, 2, 2, { "3", "4", "5" });
    engine->pushMessageQueue(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, raftMsg->append_entries_req()), 0);
    BOOST_CHECK_EQUAL(engine->getCommitIndex(), 4);
    BOOST_CHECK(cr::from(storage->getEntries(1, 5, std::numeric_limits<std::uint64_t>::max())).map([](auto&& e) {return e.value(); }).equals(cr::from({ "1", "2", "3", "4", "5" })));
    messages.clear();
}

BOOST_AUTO_TEST_SUITE_END()

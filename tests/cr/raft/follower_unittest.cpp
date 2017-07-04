﻿#include <boost/test/unit_test.hpp>

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

class SimpleStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        entries.push_back(value);
    }

    std::vector<std::string> entries;
};

struct RaftEngineFixture;

namespace cr
{
    namespace raft
    {
        template <>
        struct DebugVisitor<RaftEngineFixture>
        {
            DebugVisitor()
            {
                storage = std::make_shared<cr::raft::MemStorage>();
                engine = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&SimpleStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setElectionTimeout(std::make_pair(100, 200))
                    .setRandom(std::bind(&std::default_random_engine::operator(), &random_))
                    .build();
                engine->initialize(0);
            }

            ~DebugVisitor()
            {}

            void setVoteFor(boost::optional<std::uint32_t> voteFor)
            {
                engine->setVotedFor(voteFor);
            }

            void setLeaderId(boost::optional<std::uint32_t> leaderId)
            {
                engine->setLeaderId(leaderId);
            }

            void setCommitIndex(std::uint64_t commitIndex)
            {
                engine->setCommitIndex(commitIndex);
            }

            void setCurrentTerm(std::uint32_t currentTerm)
            {
                engine->setCurrentTerm(currentTerm);
            }

            // 校验日志复制成功
            int checkVoteSuccess(std::uint32_t candidateId, const pb::VoteReq& request)
            {
                if (messages.size() != 1)
                {
                    return 1;
                }
                if (messages[0]->msg_type() != pb::RaftMsg::VOTE_RESP || !messages[0]->has_vote_resp())
                {
                    return 2;
                }
                auto& response = messages[0]->vote_resp();
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

            int checkLogAppendSuccess(std::uint32_t leaderId, const pb::LogAppendReq& request)
            {
                if (messages.size() != 1)
                {
                    return 1;
                }
                if (!engine->getLeaderId() || *engine->getLeaderId() != leaderId)
                {
                    return 2;
                }
                if (messages[0]->msg_type() != pb::RaftMsg::LOG_APPEND_RESP || !messages[0]->has_log_append_resp())
                {
                    return 3;
                }
                auto& response = messages[0]->log_append_resp();
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
            SimpleStatMachine stateMachine;
            std::default_random_engine random_;
            std::shared_ptr<cr::raft::RaftEngine> engine;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftFollower)

BOOST_FIXTURE_TEST_CASE(electionTimeout, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    std::uint64_t nowTime = 1;
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().first - 1;
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);
}

BOOST_FIXTURE_TEST_CASE(nextElectionTimeout, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(0);
    request.set_last_log_term(1);
    request.set_candidate_term(0);

    std::uint64_t nowTime = 1;
    engine->getMessageQueue().push_back(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().first - 1;
    engine->getMessageQueue().push_back(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().first - 1;
    engine->getMessageQueue().push_back(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().first - 1;
    engine->getMessageQueue().push_back(raftMsg);
    BOOST_CHECK_LE(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);
}

// 任期为0，投票肯定失败
BOOST_FIXTURE_TEST_CASE(voteZeroTerm, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(0);
    request.set_last_log_term(0);
    request.set_candidate_term(0);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkVoteSuccess(2, request), 0);
    messages.clear();
}

// 任期为1，投票成功
BOOST_FIXTURE_TEST_CASE(voteOneTerm, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(0);
    request.set_last_log_term(0);
    request.set_candidate_term(1);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, request), 0);
    messages.clear();
}

// 任期为1，重复投票成功
BOOST_FIXTURE_TEST_CASE(voteRepeatedOneTerm, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(0);
    request.set_last_log_term(0);
    request.set_candidate_term(1);
    
    setVoteFor(2);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, request), 0);
    messages.clear();
}

// 日志不匹配，投票失败
BOOST_FIXTURE_TEST_CASE(voteLogMismatch, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    storage->append({ 1, 1, "1" });
    storage->append({ 2, 3, "2" });

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(0);
    request.set_last_log_term(0);
    request.set_candidate_term(1);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkVoteSuccess(2, request), 0);
    messages.clear();

    request.set_last_log_index(2);
    request.set_last_log_term(2);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkVoteSuccess(2, request), 0);
    messages.clear();

    request.set_last_log_index(1);
    request.set_last_log_term(3);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkVoteSuccess(2, request), 0);
    messages.clear();
}

// 日志匹配，投票成功
BOOST_FIXTURE_TEST_CASE(voteLogMatch, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    storage->append({ 1, 1, "1" });
    storage->append({ 2, 3, "2" });

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);

    auto& request = *(raftMsg->mutable_vote_req());
    request.set_last_log_index(2);
    request.set_last_log_term(3);
    request.set_candidate_term(1);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, request), 0);
    messages.clear();

    request.set_last_log_index(3);
    request.set_last_log_term(3);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, request), 0);
    messages.clear();

    request.set_last_log_index(0);
    request.set_last_log_term(4);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkVoteSuccess(2, request), 0);
    messages.clear();
}

// 任期为1，空日志，成功
BOOST_FIXTURE_TEST_CASE(logAppendOneTerm, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(1);
    request.set_leader_commit(0);
    request.set_prev_log_index(0);
    request.set_prev_log_term(0);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, request), 0);
    messages.clear();
}

// 任期为2，空日志，成功
BOOST_FIXTURE_TEST_CASE(logAppendTwoTerm, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    setLeaderId(3);
    setCurrentTerm(1);

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(2);
    request.set_leader_commit(0);
    request.set_prev_log_index(0);
    request.set_prev_log_term(0);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, request), 0);
    messages.clear();
}

// 任期比当前小，空日志，失败
BOOST_FIXTURE_TEST_CASE(logAppendTermLittle, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    setLeaderId(3);
    setCurrentTerm(2);

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(1);
    request.set_leader_commit(0);
    request.set_prev_log_index(0);
    request.set_prev_log_term(0);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkLogAppendSuccess(2, request), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->log_append_resp().follower_term(), 2);
    messages.clear();
}

// 日志不匹配
BOOST_FIXTURE_TEST_CASE(logAppendLogMismatch, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    storage->append({ 1,1,"1" });
    storage->append({ 2,2,"2" });

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(1);
    request.set_leader_commit(0);
    request.set_prev_log_index(3);
    request.set_prev_log_term(2);

    
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkLogAppendSuccess(2, request), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->log_append_resp().last_log_index(), 2);
    messages.clear();

    request.set_prev_log_index(2);
    request.set_prev_log_term(1);
    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_NE(checkLogAppendSuccess(2, request), 0);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(messages[0]->log_append_resp().last_log_index(), 1);
    BOOST_CHECK_EQUAL(storage->lastIndex(), 1);
    messages.clear();
}

// 日志不匹配
BOOST_FIXTURE_TEST_CASE(logAppendLogMatch, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    storage->append({ 1,1,"1" });
    storage->append({ 2,2,"2" });

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(1);
    request.set_leader_commit(0);
    request.set_prev_log_index(2);
    request.set_prev_log_term(2);

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, request), 0);
    messages.clear();
}

// 日志不匹配
BOOST_FIXTURE_TEST_CASE(logAppendLogThreeEntry, cr::raft::DebugVisitor<RaftEngineFixture>)
{
    storage->append({ 1,1,"1" });
    storage->append({ 2,2,"2" });
    setCurrentTerm(2);

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(2);
    request.set_leader_commit(4);
    request.set_prev_log_index(2);
    request.set_prev_log_term(2);
    request.add_entries("3");
    request.add_entries("4");
    request.add_entries("5");

    engine->getMessageQueue().push_back(raftMsg);
    engine->update(1, messages);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendSuccess(2, request), 0);
    BOOST_CHECK_EQUAL(engine->getCommitIndex(), 4);
    BOOST_CHECK(cr::from(storage->entries(1, 5)).map([](auto&& e) {return e.getValue(); }).equals(cr::from({ "1", "2", "3", "4", "5" })));
    messages.clear();
}

BOOST_AUTO_TEST_SUITE_END()

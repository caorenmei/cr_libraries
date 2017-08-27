#include <boost/test/unit_test.hpp>

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft.h>
#include <cr/raft/raft_msg.pb.h>

class CandidateStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        getEntries.push_back(value);
    }

    std::vector<std::string> getEntries;
};

struct CandidateFixture;

namespace cr
{
    namespace raft
    {
        template <>
        struct DebugVisitor<CandidateFixture>
        {
            DebugVisitor()
            {
                storage = std::make_shared<cr::raft::MemStorage>();
                raft = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4,5 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&CandidateStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setElectionTimeout(100, 100)
                    .setHeartbeatTimeout(50)
                    .setRandomSeed(0)
                    .build();
                raft->initialize(0);
            }

            ~DebugVisitor()
            {}

            auto makeEntry(std::uint64_t term, std::string value)
            {
                cr::raft::pb::Entry entry;
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

                auto& request = *(raftMsg->mutable_request_vote_req());
                request.set_last_log_index(lastLogIndex);
                request.set_last_log_term(lastLogTerm);
                request.set_candidate_term(candidateTerm);

                return raftMsg;
            }

            auto makeRequestVoteRespMsg(std::uint64_t fromNodeId, std::uint64_t destNodeId,
                std::uint64_t candidateTerm, bool success)
            {
                auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                raftMsg->set_dest_node_id(destNodeId);
                raftMsg->set_from_node_id(fromNodeId);

                auto& response = *(raftMsg->mutable_request_vote_resp());
                response.set_follower_term(candidateTerm);
                response.set_success(success);

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

            int checkVoteRequest()
            {
                if (messages.size() != raft->getBuddyNodeIds().size())
                {
                    return 1;
                }
                if (!raft->getVotedFor() || *raft->getVotedFor() != raft->getNodeId())
                {
                    return 2;
                }
                for (auto&& message : messages)
                {
                    if (message->from_node_id() != raft->getNodeId())
                    {
                        return 3;
                    }
                    if (!message->has_request_vote_req())
                    {
                        return 4;
                    }
                    auto& voteReq = message->request_vote_req();
                    if (voteReq.candidate_term() != raft->getCurrentTerm())
                    {
                        return 5;
                    }
                    if (voteReq.last_log_index() != storage->getLastIndex() || voteReq.last_log_term() != storage->getLastTerm())
                    {
                        return 6;
                    }
                }
                return 0;
            }

            cr::raft::Raft::Builder builder;
            std::vector<cr::raft::Raft::RaftMsgPtr> messages;
            std::shared_ptr<cr::raft::MemStorage> storage;
            CandidateStatMachine stateMachine;
            std::shared_ptr<cr::raft::Raft> raft;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftCandidate)

BOOST_FIXTURE_TEST_CASE(electionTimeout, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::FOLLOWER);

    nowTime = nowTime + builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::CANDIDATE);
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(checkVoteRequest(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(nextElectionTimeout, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    auto raftMsg = makeRequestVoteRespMsg(2, 1, 0, false);
    raft->pushMessageQueue(raftMsg);

    nowTime = nowTime + builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    messages.clear();

    nowTime = nowTime + 1;
    raftMsg = makeRequestVoteRespMsg(2, 1, raft->getCurrentTerm(), true);
    raft->pushMessageQueue(raftMsg);
    BOOST_CHECK_LT(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    messages.clear();

    nowTime = nowTime + builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(checkVoteRequest(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(receivesMajority, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    nowTime = nowTime + builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    messages.clear();

    auto raftMsg = makeRequestVoteRespMsg(2, 1, raft->getCurrentTerm(), true);
    raft->pushMessageQueue(raftMsg);
    nowTime = nowTime + 1;
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::CANDIDATE);

    raftMsg = makeRequestVoteRespMsg(3, 1, raft->getCurrentTerm(), false);
    raft->pushMessageQueue(raftMsg);
    nowTime = nowTime + 1;
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::CANDIDATE);

    raftMsg = makeRequestVoteRespMsg(4, 1, raft->getCurrentTerm(), true);
    raft->pushMessageQueue(raftMsg);
    nowTime = nowTime + 1;
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
}

BOOST_FIXTURE_TEST_CASE(appendEntriesReq, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    nowTime = nowTime + builder.getMaxElectionTimeout();
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime + builder.getMaxElectionTimeout());
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::CANDIDATE);
    messages.clear();

    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 0, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    nowTime = nowTime + 1;
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::CANDIDATE);

    raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    nowTime = nowTime + 1;
    BOOST_CHECK_EQUAL(raft->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::FOLLOWER);
}


BOOST_AUTO_TEST_SUITE_END()

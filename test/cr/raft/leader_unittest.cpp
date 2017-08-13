#include <boost/test/unit_test.hpp>

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/leader.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft.h>
#include <cr/raft/raft_msg.pb.h>

class LeaderStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        getEntries.push_back(value);
    }

    std::vector<std::string> getEntries;
};

struct LeaderFixture;

namespace cr
{
    namespace raft
    {
        template <>
        struct DebugVisitor<LeaderFixture>
        {
            DebugVisitor()
            {
                storage = std::make_shared<cr::raft::MemStorage>();
                raft = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4,5 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&LeaderStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setElectionTimeout(100, 200)
                    .setHeartbeatTimeout(50)
                    .setMaxWaitEntriesNum(8)
                    .setMaxPacketEntriesNum(2)
                    .setMaxPacketSize(1024)
                    .setRandomSeed(0)
                    .build();
                raft->initialize(nowTime);
            }

            ~DebugVisitor()
            {}

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

            auto makeAppendEntriesResp(std::uint64_t fromNode, std::uint64_t followerTerm, std::uint64_t lastLogIndex, bool success)
            {
                auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                raftMsg->set_dest_node_id(raft->getNodeId());
                raftMsg->set_from_node_id(fromNode);
                raftMsg->set_msg_type(cr::raft::pb::RaftMsg::APPEND_ENTRIES_RESP);

                auto& request = *(raftMsg->mutable_append_entries_resp());
                request.set_follower_term(followerTerm);
                request.set_last_log_index(lastLogIndex);
                request.set_success(success);

                return raftMsg;
            }

            void transactionLeader()
            {
                nowTime = nowTime + builder.getMaxElectionTimeout();
                raft->update(nowTime, messages);
                raft->update(nowTime, messages);
                messages.clear();

                for (auto nodeId : { 2,3 })
                {
                    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                    raftMsg->set_from_node_id(nodeId);
                    raftMsg->set_dest_node_id(1);
                    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::REQUEST_VOTE_RESP);

                    auto& request = *(raftMsg->mutable_request_vote_resp());
                    request.set_follower_term(raft->getCurrentTerm());
                    request.set_success(true);

                    raft->pushMessageQueue(raftMsg);
                    nowTime = nowTime + 1;
                    raft->update(nowTime, messages);
                }
            }

            int checkLogAppendMsg()
            {
                std::set<std::uint64_t> destNodeIds;
                for (auto&& message : messages)
                {
                    if (!raft->isBuddyNodeId(message->dest_node_id()))
                    {
                        return 1;
                    }
                    if (!destNodeIds.insert(message->dest_node_id()).second)
                    {
                        return 2;
                    }
                    if (message->msg_type() != pb::RaftMsg::APPEND_ENTRIES_REQ || !message->has_append_entries_req())
                    {
                        return 3;
                    }
                    auto& appendReq = message->append_entries_req();
                    if (appendReq.leader_commit() != raft->getCommitIndex())
                    {
                        return 4;
                    }
                    if (appendReq.leader_term() != raft->getCurrentTerm())
                    {
                        return 5;
                    }
                    if (appendReq.prev_log_index() > storage->getLastIndex() || appendReq.prev_log_term() > storage->getLastTerm())
                    {
                        return 6;
                    }
                    if (appendReq.prev_log_index() + appendReq.entries_size() > storage->getLastIndex())
                    {
                        return 7;
                    }
                }
                return 0;
            }

            int checkNextLogIndex(std::uint64_t nodeId, std::uint64_t nextLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(raft->currentState_);
                auto nodeIter = std::find_if(leader->nodes_.begin(), leader->nodes_.end(), [&](auto& node) {return node.nodeId == nodeId; });
                if (nodeIter == leader->nodes_.end())
                {
                    return 1;
                }
                if (nodeIter->nextLogIndex != nextLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            int checkMatchLogIndex(std::uint64_t nodeId, std::uint64_t matchLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(raft->currentState_);
                auto nodeIter = std::find_if(leader->nodes_.begin(), leader->nodes_.end(), [&](auto& node) {return node.nodeId == nodeId; });
                if (nodeIter == leader->nodes_.end())
                {
                    return 1;
                }
                if (nodeIter->matchLogIndex != matchLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            int checkReplyLogIndex(std::uint64_t nodeId, std::uint64_t replyLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(raft->currentState_);
                auto nodeIter = std::find_if(leader->nodes_.begin(), leader->nodes_.end(), [&](auto& node) {return node.nodeId == nodeId; });
                if (nodeIter == leader->nodes_.end())
                {
                    return 1;
                }
                if (nodeIter->replyLogIndex != replyLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            cr::raft::Raft::Builder builder;
            std::vector<cr::raft::Raft::RaftMsgPtr> messages;
            std::shared_ptr<cr::raft::MemStorage> storage;
            LeaderStatMachine stateMachine;
            std::default_random_engine random_;
            std::shared_ptr<cr::raft::Raft> raft;
            std::uint64_t nowTime = 0;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftLeader)

BOOST_FIXTURE_TEST_CASE(firstHeatbeatMsg, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
    BOOST_CHECK(messages.empty());

    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), raft->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(heatbeat, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
    BOOST_CHECK(messages.empty());

    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), raft->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    for (auto&& nodeId : raft->getBuddyNodeIds())
    {
        BOOST_CHECK_EQUAL(checkNextLogIndex(nodeId, 1), 0);
        BOOST_CHECK_EQUAL(checkMatchLogIndex(nodeId, 0), 0);
        BOOST_CHECK_EQUAL(checkReplyLogIndex(nodeId, 0), 0);
    }
    messages.clear();

    nowTime += raft->getHeatbeatTimeout() - 1;
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 0);

    nowTime += raft->getHeatbeatTimeout();
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), raft->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppend, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append(1, { makeEntry(1, "1") });
    storage->append(2, { makeEntry(1, "2") });
    storage->append(3, { makeEntry(1, "3") });
    transactionLeader();
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
    BOOST_CHECK(messages.empty());

    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), raft->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    for (auto&& nodeId : raft->getBuddyNodeIds())
    {
        BOOST_CHECK_EQUAL(checkNextLogIndex(nodeId, 4), 0);
        BOOST_CHECK_EQUAL(checkMatchLogIndex(nodeId, 0), 0);
        BOOST_CHECK_EQUAL(checkReplyLogIndex(nodeId, 3), 0);
    }
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppendNotMatch, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append(1, { makeEntry(1, "1") });
    storage->append(2, { makeEntry(1, "2") });
    storage->append(3, { makeEntry(1, "3") });
    transactionLeader();
    BOOST_REQUIRE_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
    BOOST_CHECK(messages.empty());

    raft->update(nowTime, messages);
    messages.clear();

    nowTime += 1;
    auto logAppendResp0 = makeAppendEntriesResp(2, 1, 1, false);
    raft->pushMessageQueue(logAppendResp0);
    raft->update(nowTime, messages);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_REQUIRE_EQUAL(checkLogAppendMsg(), 0);
    BOOST_REQUIRE_EQUAL(checkNextLogIndex(2, 4), 0);
    BOOST_REQUIRE_EQUAL(checkReplyLogIndex(2, 1), 0);
    auto& appendEntriesReq = messages[0]->append_entries_req();
    BOOST_CHECK_EQUAL(appendEntriesReq.prev_log_index(), 1);
    BOOST_CHECK(cr::from(appendEntriesReq.entries()).map([](auto&& e){return e.value();}).equals(cr::from({ "2", "3" })));
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppendUpdateCommit, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append(1, { makeEntry(1, "1") });
    storage->append(2, { makeEntry(1, "2") });
    storage->append(3, { makeEntry(1, "3") });
    transactionLeader();
    BOOST_REQUIRE_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
    BOOST_CHECK(messages.empty());

    raft->update(nowTime, messages);
    messages.clear();

    nowTime += 1;
    auto logAppendResp0 = makeAppendEntriesResp(2, 1, 1, false);
    raft->pushMessageQueue(logAppendResp0);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp1 = makeAppendEntriesResp(3, 1, 1, false);
    raft->pushMessageQueue(logAppendResp1);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp2 = makeAppendEntriesResp(2, 1, 3, true);
    raft->pushMessageQueue(logAppendResp2);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp3 = makeAppendEntriesResp(3, 1, 2, true);
    raft->pushMessageQueue(logAppendResp3);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCommitIndex(), 2);
    BOOST_CHECK_EQUAL(messages.size(), 4);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(RequestVoteLowTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
}

BOOST_FIXTURE_TEST_CASE(RequestVoteEqualTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 1);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
}

BOOST_FIXTURE_TEST_CASE(RequestVoteHighTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeRequestVoteReqMsg(2, 1, 0, 0, 2);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::FOLLOWER);
}

BOOST_FIXTURE_TEST_CASE(AppendEntriesLowTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 0, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
}

BOOST_FIXTURE_TEST_CASE(AppendEntriesEqualTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 1, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::LEADER);
}

BOOST_FIXTURE_TEST_CASE(AppendEntriesHighTerm, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    auto raftMsg = makeAppendEntriesReqMsg(2, 1, 2, 0, 0, 0);
    raft->pushMessageQueue(raftMsg);
    raft->update(nowTime, messages);
    BOOST_CHECK_EQUAL(raft->getCurrentState(), cr::raft::Raft::FOLLOWER);
}

BOOST_AUTO_TEST_SUITE_END()
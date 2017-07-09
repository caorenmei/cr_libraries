#include <boost/test/unit_test.hpp>

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/leader.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

class LeaderStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        entries.push_back(value);
    }

    std::vector<std::string> entries;
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
                engine = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4,5 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&LeaderStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setElectionTimeout(std::make_pair(100, 100))
                    .setHeartbeatTimeout(50)
                    .setLogWindowSize(2)
                    .setMaxPacketSize(1024)
                    .setRandom(std::bind(&std::default_random_engine::operator(), &random_))
                    .build();
                engine->initialize(nowTime);
            }

            ~DebugVisitor()
            {}

            void transactionLeader()
            {
                nowTime = nowTime + builder.getElectionTimeout().second;
                engine->update(nowTime, messages);
                engine->update(nowTime, messages);
                messages.clear();

                for (auto nodeId : { 2,3 })
                {
                    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                    raftMsg->set_from_node_id(nodeId);
                    raftMsg->set_dest_node_id(1);
                    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_RESP);

                    auto& request = *(raftMsg->mutable_vote_resp());
                    request.set_follower_term(engine->getCurrentTerm());
                    request.set_success(true);

                    engine->getMessageQueue().push_back(raftMsg);
                    nowTime = nowTime + 1;
                    engine->update(nowTime, messages);
                }
            }

            int checkLogAppendMsg()
            {
                std::set<std::uint32_t> destNodeIds;
                for (auto&& message : messages)
                {
                    if (!engine->isBuddyNodeId(message->dest_node_id()))
                    {
                        return 1;
                    }
                    if (!destNodeIds.insert(message->dest_node_id()).second)
                    {
                        return 2;
                    }
                    if (message->msg_type() != pb::RaftMsg::LOG_APPEND_REQ || !message->has_log_append_req())
                    {
                        return 3;
                    }
                    auto& appendReq = message->log_append_req();
                    if (appendReq.leader_commit() != engine->getCommitIndex())
                    {
                        return 4;
                    }
                    if (appendReq.leader_term() != engine->getCurrentTerm())
                    {
                        return 5;
                    }
                    if (appendReq.prev_log_index() > storage->lastIndex() || appendReq.prev_log_term() > storage->lastTerm())
                    {
                        return 6;
                    }
                    if (appendReq.prev_log_index() + appendReq.entries_size() > storage->lastIndex())
                    {
                        return 7;
                    }
                }
                return 0;
            }

            int checkNextLogIndex(std::uint32_t nodeId, std::uint64_t nextLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(engine->currentState_);
                if (leader->nodes_.count(nodeId) == 0)
                {
                    return 1;
                }
                if (leader->nodes_[nodeId].nextLogIndex != nextLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            int checkMatchLogIndex(std::uint32_t nodeId, std::uint64_t matchLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(engine->currentState_);
                if (leader->nodes_.count(nodeId) == 0)
                {
                    return 1;
                }
                if (leader->nodes_[nodeId].matchLogIndex != matchLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            int checkReplyLogIndex(std::uint32_t nodeId, std::uint64_t replyLogIndex)
            {
                auto leader = std::dynamic_pointer_cast<Leader>(engine->currentState_);
                if (leader->nodes_.count(nodeId) == 0)
                {
                    return 1;
                }
                if (leader->nodes_[nodeId].replyLogIndex != replyLogIndex)
                {
                    return 2;
                }
                return 0;
            }

            RaftEngine::RaftMsgPtr newLogAppendResp(std::uint32_t fromNode, std::uint32_t followerTerm, std::uint64_t lastLogIndex, bool success) 
            {
                auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
                raftMsg->set_dest_node_id(engine->getNodeId());
                raftMsg->set_from_node_id(fromNode);
                raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_RESP);

                auto& request = *(raftMsg->mutable_log_append_resp());
                request.set_follower_term(followerTerm);
                request.set_last_log_index(lastLogIndex);
                request.set_success(success);

                return raftMsg;
            }

            cr::raft::RaftEngine::Builder builder;
            std::vector<cr::raft::RaftEngine::RaftMsgPtr> messages;
            std::shared_ptr<cr::raft::MemStorage> storage;
            LeaderStatMachine stateMachine;
            std::default_random_engine random_;
            std::shared_ptr<cr::raft::RaftEngine> engine;
            std::uint64_t nowTime = 0;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftLeader)

BOOST_FIXTURE_TEST_CASE(firstHeatbeatMsg, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
    BOOST_CHECK(messages.empty());

    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), engine->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(heatbeat, cr::raft::DebugVisitor<LeaderFixture>)
{
    transactionLeader();
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
    BOOST_CHECK(messages.empty());

    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), engine->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    for (auto&& nodeId : engine->getBuddyNodeIds())
    {
        BOOST_CHECK_EQUAL(checkNextLogIndex(nodeId, 1), 0);
        BOOST_CHECK_EQUAL(checkMatchLogIndex(nodeId, 0), 0);
        BOOST_CHECK_EQUAL(checkReplyLogIndex(nodeId, 0), 0);
    }
    messages.clear();

    nowTime += engine->getHeatbeatTimeout() - 1;
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 0);

    nowTime += engine->getHeatbeatTimeout();
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), engine->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppend, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append({ 1, 1, "1" });
    storage->append({ 2, 1, "2" });
    storage->append({ 3, 1, "3" });
    transactionLeader();
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
    BOOST_CHECK(messages.empty());

    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), engine->getBuddyNodeIds().size());
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    for (auto&& nodeId : engine->getBuddyNodeIds())
    {
        BOOST_CHECK_EQUAL(checkNextLogIndex(nodeId, 4), 0);
        BOOST_CHECK_EQUAL(checkMatchLogIndex(nodeId, 0), 0);
        BOOST_CHECK_EQUAL(checkReplyLogIndex(nodeId, 3), 0);
    }
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppendNotMatch, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append({ 1, 1, "1" });
    storage->append({ 2, 1, "2" });
    storage->append({ 3, 1, "3" });
    transactionLeader();
    BOOST_REQUIRE_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
    BOOST_CHECK(messages.empty());

    engine->update(nowTime, messages);
    messages.clear();

    nowTime += 1;
    auto logAppendResp0 = newLogAppendResp(2, 1, 1, false);
    engine->getMessageQueue().push_back(logAppendResp0);
    engine->update(nowTime, messages);
    BOOST_REQUIRE_EQUAL(messages.size(), 1);
    BOOST_REQUIRE_EQUAL(checkLogAppendMsg(), 0);
    BOOST_REQUIRE_EQUAL(checkNextLogIndex(2, 4), 0);
    BOOST_REQUIRE_EQUAL(checkReplyLogIndex(2, 1), 0);
    auto& logAppendReq = messages[0]->log_append_req();
    BOOST_CHECK_EQUAL(logAppendReq.prev_log_index(), 1);
    BOOST_CHECK(cr::from(logAppendReq.entries()).equals(cr::from({ "2", "3" })));
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(logAppendUpdateCommit, cr::raft::DebugVisitor<LeaderFixture>)
{
    storage->append({ 1, 1, "1" });
    storage->append({ 2, 1, "2" });
    storage->append({ 3, 1, "3" });
    transactionLeader();
    BOOST_REQUIRE_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
    BOOST_CHECK(messages.empty());

    engine->update(nowTime, messages);
    messages.clear();

    nowTime += 1;
    auto logAppendResp0 = newLogAppendResp(2, 1, 1, false);
    engine->getMessageQueue().push_back(logAppendResp0);
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp1 = newLogAppendResp(3, 1, 1, false);
    engine->getMessageQueue().push_back(logAppendResp1);
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 1);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp2 = newLogAppendResp(2, 1, 3, true);
    engine->getMessageQueue().push_back(logAppendResp2);
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(messages.size(), 0);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();

    nowTime += 1;
    auto logAppendResp3 = newLogAppendResp(3, 1, 2, true);
    engine->getMessageQueue().push_back(logAppendResp3);
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(engine->getCommitIndex(), 2);
    BOOST_CHECK_EQUAL(messages.size(), 4);
    BOOST_CHECK_EQUAL(checkLogAppendMsg(), 0);
    messages.clear();
}

BOOST_AUTO_TEST_SUITE_END()

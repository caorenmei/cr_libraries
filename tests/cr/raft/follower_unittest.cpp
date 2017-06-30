#include <boost/test/unit_test.hpp>

#include <memory>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/follower.h>
#include <cr/raft/mem_log_storage.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

struct GetFollower;
struct SetCurrentTerm;

namespace cr
{
    namespace raft
    {
        
        template <>
        struct DebugVisitor<GetFollower>
        {
            auto get(RaftEngine& engine) const
            {
                return std::dynamic_pointer_cast<Follower>(engine.currentState_);
            }
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftFollower)

class SimpleStatMachine : public cr::raft::StateMachine
{
public:

    virtual void execute(std::uint64_t logIndex, const std::string& value, boost::any ctx) override
    {
        entries.push_back(value);
    }

    std::vector<std::string> entries;
};

struct RaftEngineFixture
{
    RaftEngineFixture()
    {
        logStrage = std::make_shared<cr::raft::MemLogStorage>();
        stateMachine = std::make_shared<SimpleStatMachine>();
        raftEngine = builder.setNodeId(1)
            .setOtherNodeIds({ 2,3,4 })
            .setLogStorage(logStrage)
            .setStateMachine(stateMachine)
            .setElectionTimeout(500)
            .setVoteTimeout(std::make_pair(100, 200))
            .build();
        raftEngine->initialize(1);
    }

    ~RaftEngineFixture()
    {}

    cr::raft::RaftEngine::Builder builder;
    std::vector<cr::raft::RaftEngine::RaftMsgPtr> outMessages;
    std::shared_ptr<cr::raft::MemLogStorage> logStrage;
    std::shared_ptr<SimpleStatMachine> stateMachine;
    std::shared_ptr<cr::raft::RaftEngine> raftEngine;
};

BOOST_FIXTURE_TEST_CASE(electionTimeout, RaftEngineFixture)
{
    raftEngine->update(1, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    raftEngine->update(builder.getElectionTimeout() / 2, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    raftEngine->update(builder.getElectionTimeout(), outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    raftEngine->update(builder.getElectionTimeout() + 1, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);
}

BOOST_FIXTURE_TEST_CASE(voteFor, RaftEngineFixture)
{
    auto follower = cr::raft::DebugVisitor<GetFollower>().get(*raftEngine);
    cr::raft::pb::RaftMsg raftMsg;
    raftMsg.set_msg_type(cr::raft::pb::RaftMsg::VOTE_REQ);
    cr::raft::pb::VoteReq voteReq;
    cr::raft::pb::VoteResp voteResp;
    raftEngine->update(1, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    // 无效消息
    voteReq.set_candidate_id(5);
    voteReq.set_candidate_term(1);
    voteReq.set_last_log_index(0);
    voteReq.set_last_log_term(0);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(2, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 0);
    BOOST_CHECK_EQUAL(follower->getLastHeartbeatTime(), 1);

    // 第一轮投票，肯定成功
    voteReq.set_candidate_id(2);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(3, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(follower->getLastHeartbeatTime(), 3);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(voteResp.success());
    BOOST_CHECK_EQUAL(voteResp.follower_id(), 1);
    BOOST_CHECK_EQUAL(voteResp.follower_term(), 1);
    BOOST_CHECK_EQUAL(*raftEngine->getVotedFor(), 2);
    outMessages.clear();

    // 已经投过票，再次投票成功
    voteReq.set_candidate_id(2);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(3, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(voteResp.success());
    outMessages.clear();

    // 已经投过票，同一任期其它节点投票失败
    voteReq.set_candidate_id(3);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(4, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(!voteResp.success());
    BOOST_CHECK_EQUAL(*raftEngine->getVotedFor(), 2);
    outMessages.clear();

    // 已经投过票，新增任期投票成功
    voteReq.set_candidate_term(2);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(5, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(voteResp.success());
    BOOST_CHECK_EQUAL(voteResp.follower_term(), 2);
    BOOST_CHECK_EQUAL(*raftEngine->getVotedFor(), 3);
    outMessages.clear();

    // 日志不够新，不能投票
    logStrage->append({ 1, 1, "0" });
    logStrage->append({ 2, 2, "0" });
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(6, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(!voteResp.success());
    outMessages.clear();

    // 任期号相同，index小，不能投票
    voteReq.set_last_log_index(1);
    voteReq.set_last_log_term(2);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(7, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(!voteResp.success());
    outMessages.clear();

    // 日志相同，可以投票
    voteReq.set_last_log_index(2);
    voteReq.set_last_log_term(2);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(7, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(voteResp.success());
    outMessages.clear();

    // 任期号更大，可以投票
    voteReq.set_last_log_index(2);
    voteReq.set_last_log_term(3);
    raftMsg.mutable_msg()->PackFrom(voteReq);
    raftEngine->pushTailMessage(std::make_shared<cr::raft::pb::RaftMsg>(raftMsg));
    raftEngine->update(7, outMessages);
    BOOST_CHECK_EQUAL(outMessages.size(), 1);
    BOOST_CHECK_EQUAL(outMessages[0]->msg_type(), cr::raft::pb::RaftMsg::VOTE_RESP);
    BOOST_CHECK(outMessages[0]->msg().UnpackTo(&voteResp));
    BOOST_CHECK(voteResp.success());
    outMessages.clear();
}

BOOST_AUTO_TEST_SUITE_END()

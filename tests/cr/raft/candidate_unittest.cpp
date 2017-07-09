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

class CandidateStatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        entries.push_back(value);
    }

    std::vector<std::string> entries;
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
                engine = builder.setNodeId(1)
                    .setBuddyNodeIds({ 2,3,4,5 })
                    .setStorage(storage)
                    .setEexcuteCallback(std::bind(&CandidateStatMachine::execute, &stateMachine, std::placeholders::_1, std::placeholders::_2))
                    .setElectionTimeout(std::make_pair(100, 100))
                    .setHeartbeatTimeout(50)
                    .setRandom(std::bind(&std::default_random_engine::operator(), &random_))
                    .build();
                engine->initialize(0);
            }

            ~DebugVisitor()
            {}

            int checkVoteRequest()
            {
                if (messages.size() != engine->getBuddyNodeIds().size())
                {
                    return 1;
                }
                if (!engine->getVotedFor() || *engine->getVotedFor() != engine->getNodeId())
                {
                    return 2;
                }
                for (auto&& message : messages)
                {
                    if (message->from_node_id() != engine->getNodeId())
                    {
                        return 3;
                    }
                    if (message->msg_type() != pb::RaftMsg::RaftMsg::VOTE_REQ || !message->has_vote_req())
                    {
                        return 4;
                    }
                    auto& voteReq = message->vote_req();
                    if (voteReq.candidate_term() != engine->getCurrentTerm())
                    {
                        return 5;
                    }
                    if (voteReq.last_log_index() != storage->lastIndex() || voteReq.last_log_term() != storage->lastTerm())
                    {
                        return 6;
                    }
                }
                return 0;
            }

            cr::raft::RaftEngine::Builder builder;
            std::vector<cr::raft::RaftEngine::RaftMsgPtr> messages;
            std::shared_ptr<cr::raft::MemStorage> storage;
            CandidateStatMachine stateMachine;
            std::default_random_engine random_;
            std::shared_ptr<cr::raft::RaftEngine> engine;
        };
    }
}

BOOST_AUTO_TEST_SUITE(RaftCandidate)

BOOST_FIXTURE_TEST_CASE(electionTimeout, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(checkVoteRequest(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(nextElectionTimeout, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_RESP);
    auto& request = *(raftMsg->mutable_vote_resp());

    raftMsg->set_from_node_id(2);
    engine->getMessageQueue().push_back(raftMsg);

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    messages.clear();

    nowTime = nowTime + 1;
    request.set_follower_term(engine->getCurrentTerm());
    request.set_success(true);
    engine->getMessageQueue().push_back(raftMsg);
    BOOST_CHECK_LT(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    messages.clear();

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(checkVoteRequest(), 0);
    messages.clear();
}

BOOST_FIXTURE_TEST_CASE(receivesMajority, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::VOTE_RESP);
    auto& request = *(raftMsg->mutable_vote_resp());

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    messages.clear();

    raftMsg->set_from_node_id(2);
    request.set_follower_term(engine->getCurrentTerm());
    request.set_success(true);
    engine->getMessageQueue().push_back(std::make_shared<cr::raft::pb::RaftMsg>(*raftMsg));
    nowTime = nowTime + 1;
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);

    raftMsg->set_from_node_id(3);
    request.set_follower_term(engine->getCurrentTerm());
    request.set_success(false);
    engine->getMessageQueue().push_back(std::make_shared<cr::raft::pb::RaftMsg>(*raftMsg));
    nowTime = nowTime + 1;
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);

    raftMsg->set_from_node_id(4);
    request.set_follower_term(engine->getCurrentTerm());
    request.set_success(true);
    engine->getMessageQueue().push_back(std::make_shared<cr::raft::pb::RaftMsg>(*raftMsg));
    nowTime = nowTime + 1;
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::LEADER);
}

BOOST_FIXTURE_TEST_CASE(logAppendReq, cr::raft::DebugVisitor<CandidateFixture>)
{
    std::uint64_t nowTime = 0;

    nowTime = nowTime + builder.getElectionTimeout().second;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime + builder.getElectionTimeout().second);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);
    messages.clear();

    auto raftMsg = std::make_shared<cr::raft::pb::RaftMsg>();
    raftMsg->set_dest_node_id(1);
    raftMsg->set_from_node_id(2);
    raftMsg->set_msg_type(cr::raft::pb::RaftMsg::LOG_APPEND_REQ);

    auto& request = *(raftMsg->mutable_log_append_req());
    request.set_leader_term(1);
    request.set_leader_commit(0);
    request.set_prev_log_index(0);
    request.set_prev_log_term(0);

    request.set_leader_term(0);
    engine->getMessageQueue().push_back(raftMsg);
    nowTime = nowTime + 1;
    engine->update(nowTime, messages);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::CANDIDATE);

    request.set_leader_term(1);
    engine->getMessageQueue().push_back(raftMsg);
    nowTime = nowTime + 1;
    BOOST_CHECK_EQUAL(engine->update(nowTime, messages), nowTime);
    BOOST_CHECK_EQUAL(engine->getMessageQueue().size(), 1);
    BOOST_CHECK_EQUAL(engine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);
}


BOOST_AUTO_TEST_SUITE_END()

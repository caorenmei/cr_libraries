#include <boost/test/unit_test.hpp>

#include <memory>
#include <vector>

#include <cr/common/assert_error.h>
#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_log_storage.h>
#include <cr/raft/raft_engine.h>

BOOST_AUTO_TEST_SUITE(RaftEngine)

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
        cr::raft::RaftEngine::Builder builder;
        logStrage = std::make_shared<cr::raft::MemLogStorage>();
        stateMachine = std::make_shared<SimpleStatMachine>();
        raftEngine = builder.setNodeId(1)
            .setOtherNodeIds({ 2,3,4 })
            .setLogStorage(logStrage)
            .setStateMachine(stateMachine)
            .setElectionTimeout(2000)
            .setVoteTimeout(std::make_pair(100, 200))
            .build();
    }

    ~RaftEngineFixture()
    {}

    std::vector<cr::raft::RaftEngine::RaftMsgPtr> outMessages;
    std::shared_ptr<cr::raft::MemLogStorage> logStrage;
    std::shared_ptr<SimpleStatMachine> stateMachine;
    std::shared_ptr<cr::raft::RaftEngine> raftEngine;
};

BOOST_AUTO_TEST_CASE(build)
{
    auto logStrage = std::make_shared<cr::raft::MemLogStorage>();
    auto stateMachine = std::make_shared<SimpleStatMachine>();
    cr::raft::RaftEngine::Builder builder;
    builder.setNodeId(1)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(logStrage)
        .setStateMachine(stateMachine)
        .setElectionTimeout(2000)
        .setVoteTimeout(std::make_pair(100, 200));
   
    BOOST_CHECK_NO_THROW(builder.build());
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 1,3,4 }) .build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 2,2,4 }).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 2,3,4 }).setLogStorage(nullptr).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setLogStorage(logStrage).setStateMachine(nullptr).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setStateMachine(stateMachine).setElectionTimeout(0).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setElectionTimeout(2000).setVoteTimeout(std::make_pair(0, 0)).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setVoteTimeout(std::make_pair(0, 200)).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setVoteTimeout(std::make_pair(200, 0)).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setVoteTimeout(std::make_pair(200, 100)).build(), cr::raft::ArgumentException);
    BOOST_CHECK_NO_THROW(builder.setVoteTimeout(std::make_pair(200, 200)).build());

    auto raftEngine = builder.setNodeId(1)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(std::make_shared<cr::raft::MemLogStorage>())
        .setStateMachine(std::make_shared<SimpleStatMachine>())
        .build();

    BOOST_CHECK_EQUAL(raftEngine->getNodeId(), 1);
    BOOST_CHECK(cr::from(raftEngine->getOtherNodeIds()).sorted().equals(cr::from({ 2,3,4 })));
    BOOST_CHECK_EQUAL(raftEngine->getCurrentTerm(), 0);
    BOOST_CHECK(!raftEngine->getVotedFor());
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
}

BOOST_FIXTURE_TEST_CASE(initialize, RaftEngineFixture)
{
    BOOST_CHECK_THROW(raftEngine->update(0, outMessages), cr::AssertError);
    BOOST_CHECK_NO_THROW(raftEngine->initialize(1));
    BOOST_CHECK_THROW(raftEngine->initialize(2), cr::AssertError);
    BOOST_CHECK_NO_THROW(raftEngine->update(0, outMessages));
}

BOOST_AUTO_TEST_SUITE_END()

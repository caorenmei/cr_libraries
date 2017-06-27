#include <boost/test/unit_test.hpp>

#include <memory>
#include <vector>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_log_storage.h>
#include <cr/raft/raft_engine.h>

BOOST_AUTO_TEST_SUITE(RaftReplay)

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
            .build();
        raftEngine->initialize();
    }

    ~RaftEngineFixture()
    {}

    std::vector<cr::raft::RaftEngine::RaftMsgPtr> outMessages;
    std::shared_ptr<cr::raft::MemLogStorage> logStrage;
    std::shared_ptr<SimpleStatMachine> stateMachine;
    std::shared_ptr<cr::raft::RaftEngine> raftEngine;
};

BOOST_FIXTURE_TEST_CASE(EmptyLog, RaftEngineFixture)
{
    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 0);

    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::REPLAY);
    raftEngine->update(0, nullptr, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 0);
}

BOOST_FIXTURE_TEST_CASE(OneLog, RaftEngineFixture)
{
    logStrage->append({ 1, 1, "0" });

    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 1);

    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::REPLAY);
    raftEngine->update(0, nullptr, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 1);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 1);

    BOOST_CHECK(cr::from(stateMachine->entries).equals(cr::from({ "0" })));
}

BOOST_FIXTURE_TEST_CASE(Three, RaftEngineFixture)
{
    logStrage->append({ 1, 1, "0" });
    logStrage->append({ 2, 1, "1" });
    logStrage->append({ 3, 1, "2" });

    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 3);

    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::REPLAY);
    raftEngine->update(0, nullptr, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::REPLAY);
    raftEngine->update(1, nullptr, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::REPLAY);
    raftEngine->update(2, nullptr, outMessages);
    BOOST_CHECK_EQUAL(raftEngine->getCurrentState(), cr::raft::RaftEngine::FOLLOWER);

    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 3);
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 3);

    BOOST_CHECK(cr::from(stateMachine->entries).equals(cr::from({ "0", "1", "2" })));
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_log_storage.h>
#include <cr/raft/raft_engine.h>

BOOST_AUTO_TEST_SUITE(RaftEngine)

class SimpleStatMachine : public cr::raft::StateMachine
{
public:

    virtual void execute(std::uint32_t instanceId, std::uint64_t logIndex, const std::string& value, boost::any ctx) override
    {
        value_ = value;
    }

    const std::string& getValue() const
    {
        return value_;
    }

private:

    std::string value_;
};

BOOST_AUTO_TEST_CASE(build)
{
    cr::raft::RaftEngine::Builder builder;
    builder.setNodeId(1)
        .setInstanceId(2)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(std::make_shared<cr::raft::MemLogStorage>())
        .setStateMachine(std::make_shared<SimpleStatMachine>());
   
    BOOST_CHECK_NO_THROW(builder.build());
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 1,3,4 }) .build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 2,2,4 }).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 2,3,4 }).setLogStorage(nullptr).build(), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(builder.setLogStorage(std::make_shared<cr::raft::MemLogStorage>()).setStateMachine(nullptr).build(), cr::raft::ArgumentException);

    auto raftEngine = builder.setNodeId(1)
        .setInstanceId(2)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(std::make_shared<cr::raft::MemLogStorage>())
        .setStateMachine(std::make_shared<SimpleStatMachine>())
        .build();

    BOOST_CHECK_EQUAL(raftEngine->getNodeId(), 1);
    BOOST_CHECK_EQUAL(raftEngine->getInstanceId(), 2);
    BOOST_CHECK(cr::from(raftEngine->getOtherNodeIds()).sorted().equals(cr::from({ 2,3,4 })));
    BOOST_CHECK_EQUAL(raftEngine->getCurrentTerm(), 0);
    BOOST_CHECK(!raftEngine->getVotedFor());
    BOOST_CHECK_EQUAL(raftEngine->getCommitLogIndex(), 0);
    BOOST_CHECK_EQUAL(raftEngine->getLastApplied(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

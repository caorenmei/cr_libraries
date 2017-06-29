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
            .setElectionTimeout(2000)
            .setVoteTimeout(std::make_pair(100, 200))
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

BOOST_AUTO_TEST_SUITE_END()

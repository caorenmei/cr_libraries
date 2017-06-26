#include <cr/raft/raft_engine_builder.h>

#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        RaftEngineBuilder::RaftEngineBuilder()
            : nodeId_(0),
            instanceId_(0)
        {}

        RaftEngineBuilder::~RaftEngineBuilder()
        {}

        RaftEngineBuilder& RaftEngineBuilder::setNodeId(std::uint32_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        std::uint32_t  RaftEngineBuilder::getNodeId() const
        {
            return nodeId_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setInstanceId(std::uint32_t instanceId)
        {
            instanceId_ = instanceId;
            return *this;
        }

        std::uint32_t RaftEngineBuilder::getInstanceId() const
        {
            return instanceId_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setOtherNodeIds(std::vector<std::uint32_t> otherNodeIds)
        {
            otherNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint32_t>& RaftEngineBuilder::getOtherNodeIds() const
        {
            return otherNodeIds_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setLogStorage(std::shared_ptr<LogStorage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        const std::shared_ptr<LogStorage>& RaftEngineBuilder::getLogStorage() const
        {
            return storage_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setStateMachine(std::shared_ptr<StateMachine> stateMachine)
        {
            stateMachine_ = std::move(stateMachine);
            return *this;
        }

        const std::shared_ptr<StateMachine>&  RaftEngineBuilder::getStateMachine() const
        {
            return stateMachine_;
        }

        std::shared_ptr<RaftEngine> RaftEngineBuilder::build()
        {
            return std::make_shared<RaftEngine>(*this);
        }
    }
}

#include <cr/raft/raft_engine.h>

#include <algorithm>

#include <cr/common/throw.h>

namespace cr
{
    namespace raft
    {
        RaftEngine::Builder::Builder()
            : nodeId_(0),
            instanceId_(0)
        {}

        RaftEngine::Builder::~Builder()
        {}

        RaftEngine::Builder& RaftEngine::Builder::setNodeId(std::uint32_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        RaftEngine::Builder& RaftEngine::Builder::setInstanceId(std::uint32_t instanceId)
        {
            instanceId_ = instanceId;
            return *this;
        }

        RaftEngine::Builder& RaftEngine::Builder::setOtherNodeIds(std::vector<std::uint32_t> otherNodeIds)
        {
            otherNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        RaftEngine::Builder& RaftEngine::Builder::setLogStorage(std::shared_ptr<LogStorage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        RaftEngine::Builder& RaftEngine::Builder::setStateMachine(std::shared_ptr<StateMachine> stateMachine)
        {
            stateMachine_ = std::move(stateMachine);
            return *this;
        }

        std::shared_ptr<RaftEngine> RaftEngine::Builder::build()
        {
            return std::make_shared<RaftEngine>(nodeId_, instanceId_, otherNodeIds_, storage_, stateMachine_);
        }

        RaftEngine::RaftEngine(std::uint32_t nodeId, std::uint32_t instanceId, std::vector<std::uint32_t> otherNodeIds, 
            std::shared_ptr<LogStorage> storage, std::shared_ptr<StateMachine> stateMachine)
            : nodeId_(nodeId),
            instanceId_(instanceId),
            otherNodeIds_(std::move(otherNodeIds)),
            storage_(std::move(storage)),
            stateMachine_(std::move(stateMachine))
        {
            // 节点有效性判断
            std::sort(otherNodeIds_.begin(), otherNodeIds_.end());
            if (std::unique(otherNodeIds_.begin(), otherNodeIds_.end()) != otherNodeIds_.end())
            {
                CR_THROW(BuildException, "Has Repeated Other Node Id");
            }
            if (std::find(otherNodeIds_.begin(), otherNodeIds_.end(), nodeId_) != otherNodeIds_.end())
            {
                CR_THROW(BuildException, "Other Node Id List Container This Id");
            }
            if (storage_ == nullptr)
            {
                CR_THROW(BuildException, "Log Storage is null");
            }
            if (stateMachine_ == nullptr)
            {
                CR_THROW(BuildException, "State Machine is null");
            }
        }

        RaftEngine::~RaftEngine()
        {}

        std::uint32_t RaftEngine::getNodeId() const
        {
            return nodeId_;
        }

        std::uint32_t RaftEngine::getInstanceId() const
        {
            return instanceId_;
        }

        const std::vector<std::uint32_t>& RaftEngine::getOtherNodeIds() const
        {
            return otherNodeIds_;
        }



    }
}
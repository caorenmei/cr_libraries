
#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_engine_builder.h>
#include <cr/raft/raft_msg.pb.h>
#include <cr/raft/mem_storage.h>


class IncreStateMachine
{
public:

    IncreStateMachine(std::uint64_t nodeId, std::shared_ptr<cr::raft::Storage> storage, std::vector<std::uint64_t> nodeIds, std::size_t randomSeed)
        : storage_(storage),
        value_(0)
    {
        nodeIds.erase(std::remove(nodeIds.begin(), nodeIds.end(), nodeId), nodeIds.end());
        cr::raft::RaftEngineBuilder builder;
        raft_ = builder.setNodeId(nodeId)
            .setBuddyNodeIds(nodeIds)
            .setStorage(storage)
            .setEexcuteCallback(std::bind(&IncreStateMachine::onExecute, this, std::placeholders::_1, std::placeholders::_2))
            .setElectionTimeout(200, 300)
            .setHeartbeatTimeout(50)
            .setMaxWaitEntriesNum(256)
            .setMaxPacketEntriesNum(128)
            .setMaxPacketSize(1024 * 1024)
            .setRandomSeed(randomSeed)
            .build();
        raft_->initialize(0);
    }

    ~IncreStateMachine()
    {}

    void onMessage(std::shared_ptr<cr::raft::pb::RaftMsg> message)
    {
        raft_->pushMessageQueue(std::move(message));
    }

    void update(std::uint64_t nowTime, std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>>& messages)
    {
        raft_->update(nowTime, messages);
        auto lastLogIndex = storage_->getLastIndex();
        if (raft_->getCurrentState() == cr::raft::RaftEngine::LEADER
            && lastLogIndex <= raft_->getCommitIndex() + 20)
        {
            std::uint64_t value = 0;
            if (lastLogIndex != 0)
            {
                auto entries = storage_->getEntries(lastLogIndex, lastLogIndex, std::numeric_limits<std::uint64_t>::max());
                value = boost::lexical_cast<std::uint64_t>(entries[0].value());
            }
            raft_->execute({ boost::lexical_cast<std::string>(value + 1) });
        }
    }

    void onExecute(std::uint64_t logIndex, const std::string& value)
    {
        std::uint64_t intValue = boost::lexical_cast<std::uint64_t>(value);
        CR_ASSERT(value_ + 1 == intValue);
        value_ = intValue;
    }

    const std::shared_ptr<cr::raft::RaftEngine>& getRaft() const
    {
        return raft_;
    }

    std::uint64_t getValue() const
    {
        return value_;
    }

private:
    std::shared_ptr<cr::raft::Storage> storage_;
    std::shared_ptr<cr::raft::RaftEngine> raft_;
    std::uint64_t value_;
};

class RaftSafetyTest
{
public:

    RaftSafetyTest()
        : lostDistribution(0.001),
        crashDistribution(0.0001),
        restartPeriod(1, 4),
        nodeIds({ 0,1,2,3,4 }),
        nowTime(0)
    {
        for (auto nodeId : nodeIds)
        {
            storages.push_back(std::make_shared<cr::raft::MemStorage>());
            logMatchingIndexs.push_back(0);
        }
        for (auto nodeId : nodeIds)
        {
            stateMachines.emplace_back(1, nullptr);
        }
    }

    void update()
    {
        nowTime = nowTime + 10;
        crashRestart();
        dispatchMessage();
        stateMachineUpdate();
        ensureElectionSafety();
        packetLost();
        stateMachineCrash();
    }

private:

    // crash 恢复
    void crashRestart()
    {
        for (auto nodeId : nodeIds)
        {
            if (stateMachines[nodeId].second == nullptr && --stateMachines[nodeId].first == 0)
            {
                stateMachines[nodeId].second = std::make_shared<IncreStateMachine>(nodeId, storages[nodeId], nodeIds, random());
            }
        }
    }

    // 派发消息
    void dispatchMessage()
    {
        for (auto&& message : messages)
        {
            auto destNodeId = message->dest_node_id();
            if (stateMachines[destNodeId].second != nullptr)
            {
                stateMachines[destNodeId].second->onMessage(message);
            }
        }
        messages.clear();
    }

    // 驱动raft
    void stateMachineUpdate()
    {
        for (auto nodeId : nodeIds)
        {
            if (stateMachines[nodeId].second != nullptr)
            {
                stateMachines[nodeId].second->update(nowTime, messages);
            }
        }
    }

    // 选举安全特性
    void ensureElectionSafety()
    {
        for (const auto& stateMachine : stateMachines)
        {
            if (stateMachine.second != nullptr && stateMachine.second->getRaft()->getCurrentState() == cr::raft::RaftEngine::LEADER)
            {
                for (const auto& stateMachine1 : stateMachines)
                {
                    if (stateMachine1.second != stateMachine.second && stateMachine1.second != nullptr && stateMachine1.second->getRaft()->getCurrentState() == cr::raft::RaftEngine::LEADER)
                    {
                        CR_ASSERT(stateMachine1.second->getRaft()->getCurrentTerm() != stateMachine.second->getRaft()->getCurrentTerm());
                    }
                }
            }
        }
    }

    // 日志匹配
    void ensureLogMatching()
    {
        
    }

    // 丢包
    void packetLost()
    {
        for (auto&& message : messages)
        {
            if (lostDistribution(random))
            {
                message.reset();
            }
        }
        messages.erase(std::remove(messages.begin(), messages.end(), nullptr), messages.end());
    }

    // crash
    void stateMachineCrash()
    {
        for (auto& test : stateMachines)
        {
            if (test.second && crashDistribution(random))
            {
                test.second.reset();
                test.first = restartPeriod(random);
            }
        }
    }

    std::default_random_engine random;
    std::bernoulli_distribution lostDistribution;
    std::bernoulli_distribution crashDistribution;
    std::uniform_int_distribution<std::size_t> restartPeriod;
    std::vector<std::uint64_t> nodeIds;
    std::vector<std::shared_ptr<cr::raft::Storage>> storages;
    std::vector<std::uint64_t> logMatchingIndexs;
    std::vector<std::pair<std::size_t, std::shared_ptr<IncreStateMachine>>> tests;
    std::vector<std::pair<std::size_t, std::shared_ptr<IncreStateMachine>>> stateMachines;
    std::uint64_t nowTime;
    std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages;
};


int main(int argc, char* argv[])
{
    RaftSafetyTest test;
    for (std::size_t i = 0; i < 100 * 10000; ++i)
    {
        test.update();
    }
}
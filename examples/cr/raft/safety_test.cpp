
#include <vector>

#include <boost/lexical_cast.hpp>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_engine_builder.h>
#include <cr/raft/raft_msg.pb.h>
#include <cr/raft/mem_storage.h>


class SafetyTest
{
public:

    SafetyTest(std::uint64_t nodeId, std::shared_ptr<cr::raft::Storage> storage, std::vector<std::uint64_t> nodeIds)
        : storage_(storage),
        value_(0)
    {
        nodeIds.erase(std::find(nodeIds.begin(), nodeIds.end(), nodeId), nodeIds.end());
        cr::raft::RaftEngineBuilder builder;
        raft_ = builder.setNodeId(nodeId)
            .setBuddyNodeIds(nodeIds)
            .setStorage(storage)
            .setEexcuteCallback(std::bind(&SafetyTest::onExecute, this, std::placeholders::_1, std::placeholders::_2))
            .setElectionTimeout(200, 300)
            .setHeartbeatTimeout(50)
            .setMaxEntriesNum(128)
            .setMaxPacketSize(1024 * 1024)
            .setRandomSeed(std::random_device()())
            .build();
        raft_->initialize(0);
    }

    ~SafetyTest()
    {}

    void onMessage(std::shared_ptr<cr::raft::pb::RaftMsg> message)
    {
        raft_->pushMessageQueue(std::move(message));
    }

    void update(std::uint64_t nowTime, std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages)
    {
        raft_->update(nowTime, messages);
        auto lastLogIndex = storage_->getLastIndex();
        if (raft_->getCurrentState() == cr::raft::RaftEngine::LEADER
            && lastLogIndex <= raft_->getCommitIndex() + 20)
        {
            std::uint64_t value = 1;
            if (lastLogIndex != 0)
            {
                auto entries = storage_->getEntries(lastLogIndex, lastLogIndex);
                value = boost::lexical_cast<std::uint64_t>(entries[0].getValue());
            }
            raft_->execute({ boost::lexical_cast<std::string>(value) });
        }
    }

    void onExecute(std::uint64_t logIndex, const std::string& value)
    {
        std::uint64_t intValue = boost::lexical_cast<std::uint64_t>(value);
        CR_ASSERT(value_ + 1 == intValue);
        value_ = intValue;
    }

private:
    std::shared_ptr<cr::raft::Storage> storage_;
    std::shared_ptr<cr::raft::RaftEngine> raft_;
    std::uint64_t value_;
};

int main(int argc, char* argv[])
{
    std::default_random_engine random;
    std::bernoulli_distribution lostDistribution(0.1);
    std::bernoulli_distribution crashDistribution(0.002);
    std::uniform_int_distribution<std::size_t> restartPeriod(1, 4);
    // 5个节点
    std::vector<std::uint64_t> nodeIds = { 0,1,2,3,4 };
    std::vector<std::shared_ptr<cr::raft::Storage>> storages;
    for (auto nodeId : nodeIds)
    {
        storages.push_back(std::make_shared<cr::raft::MemStorage>());
    }
    std::vector <std::pair<std::size_t, std::shared_ptr<SafetyTest>>> tests;
    for (auto nodeId : nodeIds)
    {
        tests.emplace_back(1, nullptr);
    }
    std::uint64_t nowTime = 0;
    std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages;
    for (std::size_t i = 0; i != 10 * 10000; ++i)
    {
        nowTime = nowTime + 10;
        // crash 恢复
        for (auto nodeId : nodeIds)
        {
            if (tests[nodeId].second == nullptr && --tests[nodeId].first == 0)
            {
                tests[nodeId].second = std::make_shared<SafetyTest>(nodeId, storages[nodeId], nodeIds);
            }
        }
        // 派发消息
        for (auto&& message : messages)
        {
            const auto& test = tests[message->dest_node_id()].second;
            if (test != nullptr && message != nullptr)
            {
                test->onMessage(std::move(message));
            }
        }
        messages.clear();
        // 驱动raft
        for (auto nodeId : nodeIds)
        {
            const auto& test = tests[nodeId].second;
            if (test != nullptr)
            {
                test->update(nowTime, messages);
            }
        }
        // 丢包
        for (auto&& message : messages)
        {
            if (lostDistribution(random))
            {
                message.reset();
            }
        }
        // crash
        for (auto& test : tests)
        {
            if (test.second && crashDistribution(random))
            {
                test.second.reset();
                test.first = restartPeriod(random);
            }
        }
    }

}
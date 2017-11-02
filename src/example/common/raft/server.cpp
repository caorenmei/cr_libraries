#include "server.h"

#include <boost/lexical_cast.hpp>

#include <cr/raft/raft_msg.pb.h>
#include <cr/raft/mem_storage.h>

Server::Server(std::uint64_t nodeId, std::vector<std::uint64_t> buddyNodeIds, std::size_t randomSeed)
    : random_(randomSeed),
    nodeId_(nodeId),
    buddyNodeIds_(buddyNodeIds),
    storage_(std::make_shared<cr::raft::MemStorage>()),
    nextCrashTime_(0),
    crashDuration_(0)
{}

Server::~Server()
{}

std::uint64_t Server::getNodeId() const
{
    return nodeId_;
}

void Server::start(std::uint64_t nowTime)
{
    nextCrashTime_ = nowTime;
    crashDuration_ = random_() / (10 * 1000);
}

void Server::receive(std::shared_ptr<cr::raft::pb::RaftMsg> message)
{
    // 5%丢包率
    if (raft_/* && (random_() / 1000 < 950)*/)
    {
        auto& state = raft_->getState();
        state.getMessages().push_back(message);
    }
}

void Server::update(std::uint64_t nowTime, std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>>& messages)
{
    // crash
    if (raft_ != nullptr && nowTime >= nextCrashTime_ && nowTime <= nextCrashTime_ + crashDuration_)
    {
        values_.clear();
        raft_.reset();
    }
    if (raft_ == nullptr && nowTime >= nextCrashTime_ + crashDuration_)
    {
        // 构造raft
        cr::raft::Options options;
        options.setNodeId(nodeId_)
            .setBuddyNodeIds(buddyNodeIds_)
            .setElectionTimeout(300, 500)
            .setHeartbeatTimeout(100)
            .setRandomSeed(random_())
            .setStorage(storage_)
            .setMaxWaitEntriesNum(3)
            .setMaxPacketSize(64);
        options.setEexcutable([this](std::uint64_t index, const std::string& value)
        {
            values_.push_back(boost::lexical_cast<std::uint64_t>(value));
        });
        raft_ = std::make_unique<cr::raft::Raft>(options);
        raft_->start(nowTime);
        // 重设crash
        nextCrashTime_ = nowTime + random_() % (10 * 60 * 1000);
        crashDuration_ = random_() % (30 * 1000);
        // 值生成时间
        genValueTime_ = nowTime;
    }
    if (raft_ != nullptr)
    {
        auto& state = raft_->getState();
        // 生成值
        if (state.isLeader() && genValueTime_ + 2 * 1000 <= nowTime)
        {
            std::vector<std::string> values;
            auto randomNum = random_() % 5 + 1;
            for (std::size_t i = 0; i != randomNum; ++i)
            {
                values.push_back(boost::lexical_cast<std::string>(random_()));
            }
            raft_->propose(values);
        }
        // update
        state.update(nowTime, messages);
        // 执行状态机
        while (raft_->execute()) continue;
    }
}

bool Server::isValid() const
{
    return raft_ != nullptr;
}

const std::vector<std::uint64_t>& Server::getValues() const
{
    return values_;
}
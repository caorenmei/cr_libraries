#include "server.h"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include <cr/raft/raft_msg.pb.h>
#include <cr/raft/mem_storage.h>

Server::Server(std::uint64_t nodeId, std::vector<std::uint64_t> buddyNodeIds, std::size_t randomSeed)
    : random_(randomSeed),
    nodeId_(nodeId),
    buddyNodeIds_(buddyNodeIds),
    storage_(std::make_shared<cr::raft::MemStorage>()),
    nextCrashTime_(0),
    crashDuration_(0),
    leader_(false),
    checkIndex_(0),
    value_(0)
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
    auto randomValue = random_() % 1000;
    if (raft_ && (randomValue < 1000))
    {
        auto& state = raft_->getState();
        state.getMessages().push_back(message);
    }
}

void Server::update(std::uint64_t nowTime, std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>>& messages)
{
    // crash
    //if (raft_ != nullptr && nowTime >= nextCrashTime_ && nowTime <= nextCrashTime_ + crashDuration_)
    //{
    //    raft_.reset();
    //    leader_ = false;
    //    checkIndex_ = 0;
    //    value_ = 0;
    //}
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
            value_ += boost::lexical_cast<std::uint64_t>(value);
        });
        raft_ = std::make_unique<cr::raft::Raft>(options);
        raft_->start(nowTime);
        // 重设crash
        nextCrashTime_ = nowTime + random_() % (10 * 60 * 1000);
        crashDuration_ = random_() % (5 * 1000);
        // 值生成时间
        genValueTime_ = nowTime;
    }
    if (raft_ != nullptr)
    {
        auto& state = raft_->getState();
        leader_ = state.isLeader();
        // 生成值
        if (leader_ && raft_->getCommitIndex() + 100 >= storage_->getLastIndex())
        {
            std::vector<std::string> values;
            auto randomNum = random_() % 20 + 1;
            for (std::size_t i = 0; i != randomNum; ++i)
            {
                values.push_back(boost::lexical_cast<std::string>(random_()));
            }
            raft_->propose(values);
        }
        // update
        state.update(nowTime, messages);
        leader_ = state.isLeader();
        // 执行状态机
        while (raft_->execute()) continue;
    }
}

bool Server::isValid() const
{
    return raft_ != nullptr;
}

// 是否是领导则
bool Server::isLeader() const
{
    return leader_;
}

std::uint64_t  Server::getCurrentTerm() const
{
    if (raft_)
    {
        return raft_->getCurrentTerm();
    }
    return 0;
}

// 获取值
const std::vector<cr::raft::pb::Entry>& Server::getEntries() const
{
    return storage_->getEntries();
}

// 获取提交日志索引
std::uint64_t Server::getCommitIndex() const
{
    if (raft_)
    {
        return raft_->getCommitIndex();
    }
    return 0;
}

// 获取校验过的日志索引
std::uint64_t Server::getCheckIndex() const
{
    return checkIndex_;
}

// 设置校验过的日志索引
void Server::setCheckIndex(std::uint64_t checkIndex)
{
    checkIndex_ = checkIndex;
}

// 计算出来的值
std::uint64_t Server::getValue() const
{
    return value_;
}
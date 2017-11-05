
#include <iostream>
#include <vector>

#include <cr/raft/raft_msg.pb.h>

#include "server.h"

#define myAssert(expression) if (!(expression)) throw std::runtime_error(#expression)

int main(int argc, char* argv[])
{
    std::uint64_t nowTime = 0;
    std::vector<std::unique_ptr<Server>> servers;
    std::default_random_engine random;
    // 构造
    servers.push_back(std::make_unique<Server>(1, std::vector<std::uint64_t>{ 2, 3, 4, 5 }, random()));
    servers.push_back(std::make_unique<Server>(2, std::vector<std::uint64_t>{ 1, 3, 4, 5 }, random()));
    servers.push_back(std::make_unique<Server>(3, std::vector<std::uint64_t>{ 1, 2, 4, 5 }, random()));
    servers.push_back(std::make_unique<Server>(4, std::vector<std::uint64_t>{ 1, 2, 3, 5 }, random()));
    servers.push_back(std::make_unique<Server>(5, std::vector<std::uint64_t>{ 1, 2, 3, 4 }, random()));
    // 比较
    std::vector<Server*> compServers;
    for (auto& server : servers)
    {
        compServers.push_back(server.get());
    }
    // 启动
    for (auto& server : servers)
    {
        nowTime += 10;
        server->start(nowTime);
    }
    // 循环
    std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages;
    while (true)
    {
        nowTime += 10;
        // update
        for (auto& server : servers)
        {
            server->update(nowTime, messages);
        }
        // 传输消息
        for (auto& message : messages)
        {
            auto iter = std::find_if(servers.begin(), servers.end(), [&](const std::unique_ptr<Server>& e)
            {
                return e->getNodeId() == message->dest_node_id();
            });
            // 丢包
            if (random() % 1000 < 900)
            {
                (*iter)->receive(message);
            }
        }
        messages.clear();
        // crash
        for (auto& server : servers)
        {
            std::bernoulli_distribution distriute(0.01 / (60 * 10));
            if (distriute(random))
            {
                server->crash(nowTime, 10 * 1000);
            }
        }
        // 安全特性
        auto validIter = std::partition(compServers.begin(), compServers.end(), [](const Server* lhs)
        {
            return lhs->isValid();
        });
        // 选举安全性
        std::sort(compServers.begin(), std::partition(compServers.begin(), validIter, [](const Server* lhs)
        {
            return lhs->isLeader();
        }), [](const Server* lhs, const Server* rhs)
        {
            return lhs->getCurrentTerm() < rhs->getCurrentTerm();
        });
        for (std::size_t i = 1; i < compServers.size() && compServers[i - 1]->isLeader() && compServers[i]->isLeader(); ++i)
        {
            auto server1 = compServers[i - 1];
            auto server2 = compServers[i];
            myAssert(server1->getCurrentTerm() != server2->getCurrentTerm());
        }
        // 日志匹配
        std::sort(compServers.begin(), validIter, [](const Server* lhs, const Server* rhs)
        {
            return std::make_tuple(lhs->getCheckIndex(), lhs->getCommitIndex()) < std::make_tuple(rhs->getCheckIndex(), rhs->getCommitIndex());
        });
        for (std::size_t i = 1; i < compServers.size() && compServers[i - 1]->isValid() && compServers[i]->isValid(); ++i)
        {
            auto& entries1 = compServers[i - 1]->getEntries();
            auto& entries2 = compServers[i]->getEntries();
            auto checkIndex = std::min(compServers[i - 1]->getCheckIndex(), compServers[i]->getCheckIndex());
            auto commitIndex = std::min(compServers[i - 1]->getCommitIndex(), compServers[i]->getCommitIndex());
            for (; checkIndex < commitIndex; ++checkIndex)
            {
                auto& entry1 = entries1[checkIndex];
                auto& entry2 = entries2[checkIndex];
                myAssert(entry1.index() == entry2.index() && entry1.term() == entry2.term() && entry1.value() == entry2.value());
            }
            compServers[i - 1]->setCheckIndex(checkIndex);
            compServers[i]->setCheckIndex(checkIndex);
        }
        // Leader完备性
        auto leaderIter = std::stable_partition(compServers.begin(), validIter, [](const Server* lhs)
        {
            return lhs->isLeader();
        });
        for (auto iter1 = compServers.begin(); iter1 != validIter; ++iter1)
        {
            auto server1 = *iter1;
            for (auto iter2 = compServers.begin(); iter2 != leaderIter; ++iter2)
            {
                auto server2 = *iter1;
                if (server1->getCurrentTerm() <= server2->getCurrentTerm())
                {
                    myAssert(server1->getCommitIndex() <= server2->getCommitIndex());
                }
            }
        }
    }
}
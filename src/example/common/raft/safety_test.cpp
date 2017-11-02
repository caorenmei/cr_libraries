
#include <iostream>
#include <vector>

#include <cr/raft/raft_msg.pb.h>

#include "server.h"

int main(int argc, char* argv[])
{
    std::uint64_t nowTime = 0;
    std::vector<std::unique_ptr<Server>> servers;
    // 构造
    servers.push_back(std::make_unique<Server>(1, std::vector<std::uint64_t>{ 2, 3, 4, 5 }, 101));
    servers.push_back(std::make_unique<Server>(2, std::vector<std::uint64_t>{ 1, 3, 4, 5 }, 102));
    servers.push_back(std::make_unique<Server>(3, std::vector<std::uint64_t>{ 1, 2, 4, 5 }, 103));
    servers.push_back(std::make_unique<Server>(4, std::vector<std::uint64_t>{ 1, 2, 3, 5 }, 104));
    servers.push_back(std::make_unique<Server>(5, std::vector<std::uint64_t>{ 1, 2, 3, 4 }, 105));
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
            (*iter)->receive(message);
        }
        messages.clear();
        // 比较值
        std::sort(compServers.begin(), compServers.end(), [](const Server* lhs, const Server* rhs)
        {
            return lhs->getValues().size() < rhs->getValues().size();
        });
        for (std::size_t i = 2; i < compServers.size(); ++i)
        {
            auto& values1 = compServers[i - 1]->getValues();
            auto& values2 = compServers[i]->getValues();
            assert(std::equal(values1.begin(), values1.end(), values2.begin()));
        }
    }
}
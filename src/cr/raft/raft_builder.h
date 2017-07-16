#ifndef CR_RAFT_RAFT_BUILDER_H_
#define CR_RAFT_RAFT_BUILDER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include <boost/optional.hpp>

#include <cr/raft/storage.h>

namespace cr
{
    namespace raft
    {
        /** Raft算法引擎 */
        class Raft;

        /** Raft引擎构造器 */
        class RaftBuilder
        {
        public:

            RaftBuilder();

            ~RaftBuilder();

            // 设置 Raft Id
            RaftBuilder& setNodeId(std::uint64_t nodeId);

            // 获取 Raft Id
            std::uint64_t getNodeId() const;

            // 设置其它节点的 Raft Id
            RaftBuilder& setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds);

            // 获取其它节点的 Raft Id
            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            // 设置日志存储
            RaftBuilder& setStorage(std::shared_ptr<Storage> storage);

            // 获取日志存储
            const std::shared_ptr<Storage>& getStorage() const;

            // 设置状态机执行接口
            RaftBuilder& setEexcuteCallback(std::function<void(std::uint64_t, const std::string&)> cb);

            // 获取状态机执行接口
            const std::function<void(std::uint64_t, const std::string&)>& getEexcuteCallback() const;

            // 设置随机数种子
            RaftBuilder& setRandomSeed(std::size_t seed);

            // 获取随机数种子
            std::size_t getRandomSeed() const;

            // 设置选举超时时间范围，毫秒
            RaftBuilder& setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout);

            // 获取最小选举超时时间，毫秒
            std::uint64_t getMinElectionTimeout() const;

            // 获取最大选举超时时间，毫秒
            std::uint64_t getMaxElectionTimeout() const;

            // 设置心跳超时时间，毫秒
            RaftBuilder& setHeartbeatTimeout(std::uint64_t heartbeatTimeout);

            // 获取心跳超时时间，毫秒
            std::uint64_t getHeatbeatTimeout() const;

            // 设置传输等待最大日志条目
            RaftBuilder& setMaxWaitEntriesNum(std::uint64_t maxWaitEntriesNum);

            // 获取传输等待最大日志条目
            std::uint64_t getMaxWaitEntriesNum() const;

            // 设置一次传递的最大日志条目
            RaftBuilder& setMaxPacketEntriesNum(std::uint64_t maxPacketEntriesNum);

            // 获取一次传递的最大日志条目
            std::uint64_t getMaxPacketEntriesNum() const;

            // 设置一次传递的最大日志大小
            RaftBuilder& setMaxPacketSize(std::uint64_t maxPacketSize);

            // 获取一次传递的最大日志大小
            std::uint64_t getMaxPacketLength() const;

            // 构造
            std::shared_ptr<Raft> build();

        private:

            // Raft Id
            std::uint64_t nodeId_;
            // 其它节点的 Raft Id
            std::vector<std::uint64_t> buddyNodeIds_;
            // 日志存储
            std::shared_ptr<Storage> storage_;
            // 状态机执行接口
            std::function<void(std::uint64_t, const std::string&)> executable_;
            // 随机数种子
            std::size_t randomSeed_;
            // 最小选举超时时间， 毫秒
            std::uint64_t minElectionTimeout_;
            // 最大选举超时时间，毫秒
            std::uint64_t maxElectionTimeout_;
            // 心跳超时时间，毫秒
            std::uint64_t heartbeatTimeout_;
            // 等待应答的最大日志条目
            std::uint64_t maxWaitEntriesNum_;
            // 一次传递的最大日志条目数目
            std::uint64_t maxPacketEntriesNum_;
            // 一次传递的最大日志大小
            std::uint64_t maxPacketLength_;
        };
    }
}

#endif



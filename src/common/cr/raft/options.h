#ifndef CR_COMMON_RAFT_OPTIONS_H_
#define CR_COMMON_RAFT_OPTIONS_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "storage.h"

namespace cr
{
    namespace raft
    {
        /** raft 参数 */
        class Options
        {
        public:

            /** 构造函数*/
            Options();

            /** 析构函数 */
            ~Options();

            /**
             * 设置节点Id
             * @param nodeId 节点Id
             * @return this
             */
            Options& setNodeId(std::uint64_t nodeId);

            /**
             * 获取节点Id
             * @reutrn 节点Id
             */
            std::uint64_t getNodeId() const;

            /**
             * 设置伙伴节点Id
             * @param 伙伴节点Id
             * @return this
             */
            Options& setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds);

            /**
             * 获取伙伴节点Id
             * @return 伙伴节点Id
             */
            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            /**
             * 设置存储接口
             * @param storage 存储接口
             * @return this
             */
            Options& setStorage(std::shared_ptr<Storage> storage);

            /**
             * 获取存储接口
             * @return 存储接口
             */
            const std::shared_ptr<Storage>& getStorage() const;

            /**
             * 设置日志执行接口
             * @param executable 日志执行接口
             * @return this
             */
            Options& setEexcutable(std::function<void(std::uint64_t, const std::string&)> executable);

            /**
             * 获取日志执行接口
             * @return 日志执行接口
             */
            const std::function<void(std::uint64_t, const std::string&)>& getEexcutable() const;

            /**
             * 设置随机种子
             * @param seed 随机种子
             * @return this
             */
            Options& setRandomSeed(std::size_t seed);

            /**
             * 获取随机种子
             * @return 随机种子
             */
            std::size_t getRandomSeed() const;

            /**
             * 设置选举超时时间, 毫秒
             * @param minElectionTimeout 最小选举超时时间
             * @param maxElectionTimeout 最大选举超时时间
             * @return this
             */
            Options& setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout);

            /**
             * 获取最小选举超时时间
             * @return 最小选举超时时间
             */
            std::uint64_t getMinElectionTimeout() const;

            /**
             * 获取最大选举超时时间
             * @return 最大选举超时时间
             */
            std::uint64_t getMaxElectionTimeout() const;

            /**
             * 设置心跳超时
             * @param heartbeatTimeout 心跳超时
             * @return this
             */
            Options& setHeartbeatTimeout(std::uint64_t heartbeatTimeout);

            /**
             * 获取心跳超时
             * @return 心跳超时
             */
            std::uint64_t getHeatbeatTimeout() const;

            /**
             * 设置传输最大日志条
             * @param maxWaitEntriesNum 传输最大日志条目
             * @return this
             */
            Options& setMaxWaitEntriesNum(std::uint64_t maxWaitEntriesNum);

            /**
             * 获取传输最大日志条目
             * @return 传输最大日志条目
             */
            std::uint64_t getMaxWaitEntriesNum() const;

            /**
             * 设置传输包的最大大小
             * @param maxPacketSize 传输包的最大大小
             * @return this
             */
            Options& setMaxPacketSize(std::uint64_t maxPacketSize);

            /**
             * 获取传输包的最大大小
             * @return 传输包的最大大小
             */
            std::uint64_t getMaxPacketLength() const;

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
            // 一次传递的最大日志大小
            std::uint64_t maxPacketLength_;
        };
    }
}

#endif
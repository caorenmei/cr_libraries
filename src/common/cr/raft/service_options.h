#ifndef CR_COMMON_RAFT_SERVICE_OPTIONS_H_
#define CR_COMMON_RAFT_SERVICE_OPTIONS_H_

#include <cstdint>
#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace cr
{
    namespace raft
    {
        /** Raft服务参数 */
        class ServiceOptions
        {
        public:

            /** 构造函数 */
            ServiceOptions();

            /** 析构函数 */
            ~ServiceOptions();

            /**
             * 设置自己的Id
             * @param myId 自己的Id
             */
            void setMyId(std::uint64_t myId);

            /**
             * 获取自己的Id
             * @return 自己的Id
             */
            std::uint64_t getMyId() const;

            /**
             * 设置日志保存目录
             * @param binLogPath 日志保存目录
             */
            void setBinLogPath(std::string binLogPath);

            /**
             * 获取日志保存目录
             * @return 日志保存目录
             */
            const std::string& getBinLogPath() const;

            /**
             * 获取我的地址
             * @return 我的地址
             */
            const boost::asio::ip::tcp::endpoint& getMyEndpoint() const;

            /**
             * 添加节点
             * @param id 节点Id
             * @param endpoint 地址
             */
            void addNode(std::uint64_t id, const boost::asio::ip::tcp::endpoint& endpoint);

            /**
             * 获取节点列表
             * @return 节点列表
             */
            const std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>>& getNodes() const;

            /**
             * 获取其他节点Id
             * @return 其他节点Id列表
             */
            std::vector<std::uint64_t> getBuddyNodeIds() const;

            /**
             * 获取其他节点列表
             */
            std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>> getBuddyNodes() const;

            /**
             * 设置选举超时时间
             * @param minElectionTime 最小选举超时时间
             * @param maxElectionTime 最大选举超时时间
             */
            void setElectionTime(std::uint64_t minElectionTime, std::uint64_t maxElectionTime);

            /**
             * 获取最小选举超时时间
             * @return 最小选举超时时间
             * @param maxElectionTime 最大选举超时时间
             */
            std::uint64_t getMinElectionTime() const;

            /**
             * 获取最大选举超时时间
             * @return 最大选举超时时间
             */
            std::uint64_t getMaxElectionTime() const;

            /**
             * 设置心跳时间
             * @param heartbeatTime 心跳时间
             */
            void setHeartbeatTime(std::uint64_t heartbeatTime);

            /**
             * 获取心跳时间
             * @return 心跳时间
             */
            std::uint64_t getHeartbeatTime() const;

        private:

            // 节点列表
            std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>> nodes_;
            // 自己的Id
            std::uint64_t myId_;
            // 最小选举超时时间
            std::uint64_t minElectionTime_;
            // 最大选举超时时间
            std::uint64_t maxElectionTime_;
            // 心跳时间
            std::uint64_t heartbeatTime_;
            // 日志路径
            std::string binLogPath_;

        };
    }
}

#endif

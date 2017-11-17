#ifndef CR_COMMON_RAFT_SERVICE_OPTIONS_H_
#define CR_COMMON_RAFT_SERVICE_OPTIONS_H_

#include <cstdint>
#include <string>
#include <vector>

namespace cr
{
    namespace raft
    {
        /** Raft服务参数 */
        struct ServiceOptions
        {
            /** 服务列表,格式: tcp://host:port/id -> tcp://127.0.0.1:3348/1 */
            std::vector<std::string> servers;
            /** 自己Id */
            std::uint64_t myId;
            /** 最小选举超时时间 */
            std::uint64_t minElectionTime;
            /** 最大选举超时时间 */
            std::uint64_t maxElectionTime;
            /** 心跳时间 */
            std::uint64_t heatbeatTime;
            // 日志路径
            std::string binLogPath;
        };
    }
}

#endif

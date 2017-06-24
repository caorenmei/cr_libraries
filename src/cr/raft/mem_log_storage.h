#ifndef CR_RAFT_MEM_LOG_STORAGE_H_
#define CR_RAFT_MEM_LOG_STORAGE_H_

#include <unordered_map>
#include <vector>

#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        /** 内存日志存储接口，主要用于测试 */
        class MemLogStorage : public LogStorage
        {
        public:

            /** 构造函数 */
            MemLogStorage();

            /** 析构函数 */
            ~MemLogStorage();

            MemLogStorage(const MemLogStorage&) = delete;
            MemLogStorage& operator=(const MemLogStorage&) = delete;

            /** 
             * 获取所有的实例ID
             * @param [out] instanceIds 实例ID列表
             * @return 操作结果
             */
            virtual Result getAllInstanceId(std::vector<std::uint32_t>& instanceIds) override;

            /**
             * 获取一条日志
             * @param instanceId 一个raft实例ID
             * @param logIndex 日志ID，从0开始
             * @param [out] logEntry 日志条目
             * @return 操作结果
             */
            virtual Result get(std::uint32_t instanceId, std::uint64_t logIndex, LogEntry& logEntry) override;

            /**
             * 追加日志
             * @param logEntry 日志
             * @return 操作结果
             */
            virtual Result append(const LogEntry& logEntry) override;

            /**
             * 删除日志
             * @param instanceId 实例ID
             * @param startLogIndex 起始日志索引
             * @return 操作结果
             */
            virtual Result del(std::uint32_t instanceId, std::uint64_t startLogIndex) override;

            /**
             * 删除实例的所有日志
             * @param instanceId 实例ID
             * @return 操作结果
             */
            virtual Result del(std::uint32_t instanceId) override;

            /**
             * 获取最后的日志索引
             * @param instanceId 实例ID
             * @param lastLogIndex 最后的日志索引
             * @return 操作结果
             */
            virtual Result getLastLogIndex(std::uint32_t instanceId, std::uint64_t& lastLogIndex) override;

        private:

            using LogEntries = std::vector<std::pair<std::uint32_t, std::string>>;
            std::unordered_map<std::uint32_t, LogEntries> logs_;
        };
    }
}

#endif

#ifndef CR_RAFT_MEM_LOG_STORAGE_H_
#define CR_RAFT_MEM_LOG_STORAGE_H_

#include <unordered_map>
#include <vector>

#include <cr/raft/storage.h>

namespace cr
{
    namespace raft
    {
        /** 内存日志存储接口，主要用于测试 */
        class MemStorage : public Storage
        {
        public:

            MemStorage() = default;

            MemStorage(const MemStorage&) = delete;
            MemStorage& operator=(const MemStorage&) = delete;

            virtual void append(const std::vector<Entry>& entry) override;

            virtual void remove(std::uint64_t startIndex) override;

            virtual std::vector<Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex) override;

            virtual std::uint64_t getTermByIndex(std::uint64_t index) override;

            virtual std::uint64_t getLastIndex() override;

            virtual std::uint64_t getLastTerm() override;
        private:

            std::vector<Entry> entries_;
        };
    }
}

#endif

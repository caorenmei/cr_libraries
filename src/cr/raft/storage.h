#ifndef CR_RAFT_STORAGE_H_
#define CR_RAFT_STORAGE_H_

#include <cstdint>
#include <string>
#include <vector>

#include <cr/raft/entry.h>

namespace cr
{
    namespace raft
    {
        class Storage
        {
        public:

            Storage() = default;

            virtual ~Storage() = default;

            virtual void append(const std::vector<Entry>& entries) = 0;

            virtual void remove(std::uint64_t startIndex) = 0;

            virtual std::vector<Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex) = 0;

            virtual std::uint64_t getTermByIndex(std::uint64_t index) = 0;

            virtual std::uint64_t getLastIndex() = 0;

            virtual std::uint64_t getLastTerm() = 0;
        };
    }
}

#endif

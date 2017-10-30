#ifndef CR_RAFT_STORAGE_H_
#define CR_RAFT_STORAGE_H_

#include <cstdint>
#include <string>
#include <vector>

namespace cr
{
    namespace raft
    {
        namespace pb
        {
            class Entry;
        }

        class Storage
        {
        public:

            Storage() = default;

            virtual ~Storage() = default;

            virtual void append(std::uint64_t startIndex, const std::vector<pb::Entry>& entries) = 0;

            virtual void remove(std::uint64_t startIndex) = 0;

            virtual std::vector<pb::Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) = 0;

            virtual std::uint64_t getTermByIndex(std::uint64_t index) = 0;

            virtual std::uint64_t getLastIndex() = 0;

            virtual std::uint64_t getLastTerm() = 0;
        };
    }
}

#endif

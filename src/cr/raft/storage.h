#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <string>
#include <vector>

namespace cr
{
    namespace raft
    {
        class Entry
        {
        public:

            Entry();

            Entry(std::uint64_t index, std::uint64_t getTermByIndex, std::string data);

            std::uint64_t getIndex() const;

            Entry& setIndex(std::uint64_t index);

            std::uint64_t getTerm() const;

            Entry& setTerm(std::uint64_t getTermByIndex);

            const std::string& getValue() const;

            std::string& getValue();

            Entry& setValue(std::string data);

        private:
            std::uint64_t index_;
            std::uint64_t term_;
            std::string value_;
        };

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

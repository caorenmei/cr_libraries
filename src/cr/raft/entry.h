#ifndef CR_RAFT_ENTRY_H_
#define CR_RAFT_ENTRY_H_

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

            Entry& setTerm(std::uint64_t term);

            const std::string& getValue() const;

            std::string& getValue();

            Entry& setValue(std::string value);

        private:
            std::uint64_t index_;
            std::uint64_t term_;
            std::string value_;
        };
    }
}

#endif
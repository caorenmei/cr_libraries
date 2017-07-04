#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <functional>
#include <memory>
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

            Entry(std::uint64_t index, std::uint32_t term, std::string data);

            std::uint64_t getIndex() const;

            Entry& setIndex(std::uint64_t index);

            std::uint32_t getTerm() const;

            Entry& setTerm(std::uint32_t term);

            const std::string& getValue() const;

            std::string& getValue();

            Entry& setValue(std::string data);

        private:
            std::uint64_t index_;
            std::uint32_t term_;
            std::string value_;
        };

        class Storage
        {
        public:

            Storage() {}

            virtual ~Storage() {}

            virtual void append(const Entry& entry) = 0;

            virtual void remove(std::uint64_t startIndex) = 0;

            virtual std::vector<Entry> entries(std::uint64_t startIndex, std::uint64_t stopIndex) = 0;

            virtual std::uint32_t term(std::uint64_t index) = 0;

            virtual std::uint64_t lastIndex() = 0;

            virtual std::uint32_t lastTerm() = 0;
        };
    }
}

#endif

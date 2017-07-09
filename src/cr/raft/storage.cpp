#include <cr/raft/storage.h>

namespace cr
{
    namespace raft
    {
        Entry::Entry()
            : Entry(0, 0, "")
        {}

        Entry::Entry(std::uint64_t index, std::uint64_t getTermByIndex, std::string data)
            : index_(index),
            term_(getTermByIndex),
            value_(std::move(data))
        {}

        std::uint64_t Entry::getIndex() const
        {
            return index_;
        }

        Entry&  Entry::setIndex(std::uint64_t index)
        {
            index_ = index;
            return *this;
        }

        std::uint64_t Entry::getTerm() const
        {
            return term_;
        }

        Entry& Entry::setTerm(std::uint64_t getTermByIndex)
        {
            term_ = getTermByIndex;
            return *this;
        }

        const std::string& Entry::getValue() const
        {
            return value_;
        }

        std::string& Entry::getValue()
        {
            return value_;
        }

        Entry& Entry::setValue(std::string data)
        {
            data = std::move(data);
            return *this;
        }
    }
}
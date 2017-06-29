#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        LogEntry::LogEntry()
            : LogEntry(0, 0, "")
        {}

        LogEntry::LogEntry(std::uint64_t index, std::uint32_t term, std::string value)
            : index_(index),
            term_(term),
            value_(std::move(value))
        {}

        LogEntry::~LogEntry()
        {}

        std::uint64_t LogEntry::getIndex() const
        {
            return index_;
        }

        void LogEntry::setIndex(std::uint64_t index)
        {
            index_ = index;
        }

        std::uint32_t LogEntry::getTermByIndex() const
        {
            return term_;
        }

        void LogEntry::setTerm(std::uint32_t term)
        {
            term_ = term;
        }

        const std::string& LogEntry::getValue() const
        {
            return value_;
        }

        void LogEntry::setValue(std::string value)
        {
            value_ = std::move(value);
        }

        LogStorage::LogStorage() 
        {}

        LogStorage::~LogStorage() 
        {}
    }
}
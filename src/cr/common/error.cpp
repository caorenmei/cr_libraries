#include <cr/common/error.h>

#include <utility>

namespace cr
{
    Error::Error(std::string message/* = ""*/)
        : message_(std::move(message)),
        sourceName_(""),
        sourceLine_(0)
    {}

    Error::~Error()
    {}

    const char* Error::what() const
    {
        return message_.c_str();
    }

    const std::string& Error::getMessage() const
    {
        return message_;
    }

    void Error::setSourceName(const char* sourceName)
    {
        sourceName_ = sourceName;
    }

    const char* Error::getSourceName() const
    {
        return sourceName_;
    }

    void Error::setSourceLine(int sourceLine)
    {
        sourceLine_ = sourceLine;
    }

    int Error::getSourceLine() const
    {
        return sourceLine_;
    }
}
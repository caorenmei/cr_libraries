#include <cr/common/exception.h>

#include <cstring>
#include <utility>

namespace cr
{
    Exception::Exception(std::string message/* = ""*/)
        : message_(std::move(message)),
        sourceName_(""),
		sourceLine_(0)
    {}

	Exception::~Exception()
	{}

	const char* Exception::what() const
	{
		return message_.c_str();
	}

    const std::string& Exception::getMessage() const
    {
        return message_;
    }

	void Exception::setSourceName(const char* sourceName)
	{
		sourceName_ = sourceName;
	}

	const char* Exception::getSourceName() const
	{
		return sourceName_;
	}

	void Exception::setSourceLine(int sourceLine)
	{
		sourceLine_ = sourceLine;
	}

	int Exception::getSourceLine() const
	{
		return sourceLine_;
	}
}
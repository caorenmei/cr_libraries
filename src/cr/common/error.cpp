#include <cr/common/error.h>

#include <cstring>
#include <utility>

namespace cr
{
	Error::Error(const char* message/* = ""*/)
		: sourceName_(""),
		sourceLine_(0)
	{
		std::size_t messageLength = std::strlen(message);
		message_ = new char[messageLength + 1];
		std::strncpy(message_, message, messageLength + 1);
	}

	Error::Error(const Error& other)
		: sourceName_(other.sourceName_),
		sourceLine_(other.sourceLine_)
	{
		std::size_t messageLength = std::strlen(other.message_);
		message_ = new char[messageLength + 1];
		std::strncpy(message_, other.message_, messageLength + 1);
	}

	Error::~Error()
	{
		delete [] message_;
	}

	Error& Error::operator=(const Error& other)
	{
		Error temp(other);
		std::swap(message_, temp.message_);
		return *this;
	}

	const char* Error::what() const
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
#include <cr/common/exception.h>

#include <cstring>
#include <utility>

namespace cr
{
	Exception::Exception(const char* message/* = ""*/)
		: sourceName_(""),
		sourceLine_(0)
	{
		std::size_t messageLength = std::strlen(message);
		message_ = new char[messageLength + 1];
		std::strncpy(message_, message, messageLength + 1);
	}

	Exception::Exception(const Exception& other)
		: sourceName_(other.sourceName_),
		sourceLine_(other.sourceLine_)
	{
		std::size_t messageLength = std::strlen(other.message_);
		message_ = new char[messageLength + 1];
		std::strncpy(message_, other.message_, messageLength + 1);
	}

	Exception::~Exception()
	{
		delete[] message_;
	}

	Exception& Exception::operator=(const Exception& other)
	{
		Exception temp(other);
		std::swap(message_, temp.message_);
		return *this;
	}

	const char* Exception::what() const
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
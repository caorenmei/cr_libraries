#include <cr/common/assert_builder.h>

namespace cr
{
	ThrowAssertError::ThrowAssertError(const AssertBuilder& builder)
		: builder_(builder)
	{}

	ThrowAssertError::operator int()
	{
		builder_.handler_(builder_.message_.str().c_str());
		return AssertBuilder::sAssertCount + 1;
	}

	int AssertBuilder::sAssertCount = 0;

	AssertBuilder::AssertBuilder(std::function<void(const char*)> handler, const char* file, int line, const char* expression)
		: cr_assert_impl_a(*this),
		cr_assert_impl_b(*this),
		handler_(std::move(handler))
	{
		message_ << "Failed: " << expression << "\n"
			<< "File: " << file << " Line: " << line << "\n"
			<< "Context Variables:\n";
	}
}
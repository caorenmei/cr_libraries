#ifndef CR_COMMON_ASSERT_BUILDER_H_
#define CR_COMMON_ASSERT_BUILDER_H_

#include <functional>
#include <sstream>

namespace cr
{

    template <typename E>
    class AssertBuilder
    {
    public:

        AssertBuilder& CR_ASSERT_IMPL_A;
        AssertBuilder& CR_ASSERT_IMPL_B;

        AssertBuilder(const char* expression, const char* file, int line)
            : CR_ASSERT_IMPL_A(*this),
            CR_ASSERT_IMPL_B(*this),
            sourceName_(file),
            sourceLine_(line)
        {
            message_ << "Failed: " << expression << "\n"
                << "File: " << file << " Line: " << line << "\n"
                << "Context Variables:\n";
        }

        template <typename T>
        AssertBuilder& print(const T& x, const char* varName)
        {
            message_ << "\t" << varName << " = " << x << "\n";
            return *this;
        }

        operator E() const
        {
            E e(message_.str().c_str());
            e.setSourceName(sourceName_);
            e.setSourceLine(sourceLine_);
            return e;
        }

    private:

        std::stringstream message_;
        const char* sourceName_;
        int sourceLine_;
    };
}

#endif
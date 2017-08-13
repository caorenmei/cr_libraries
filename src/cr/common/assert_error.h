#ifndef CR_COMMON_ASSERT_ERROR_H_
#define CR_COMMON_ASSERT_ERROR_H_

#include <string>

#include "error.h"

namespace cr
{

    /** 断言错误 */
    class AssertError : public Error
    {
    public:
        using Error::Error;
    };
}

#endif

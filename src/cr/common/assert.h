#ifndef CR_COMMON_ASSERT_H_
#define CR_COMMON_ASSERT_H_

#include <cr/common/assert_builder.h>
#include <cr/common/assert_error.h>
#include <cr/common/throw.h>

#define CR_ASSERT_IMPL_A(x) CR_ASSERT_IMPL_OP(x, B)
#define CR_ASSERT_IMPL_B(x) CR_ASSERT_IMPL_OP(x, A)
#define CR_ASSERT_IMPL_OP(x, next) print((x), #x).CR_ASSERT_IMPL_##next

/**
 * 安全的断言，断言失败抛出AssertError异常.
 *
 * @see cr::AssertError
 * @param 断言表达式.
 */
#define CR_ASSERT(expression)\
    if (!(expression))\
        cr::AssertBuilder::sAssertCount = (cr::AssertBuilder(\
        [](const char* message)\
        { \
            CR_THROW(cr::AssertError, message); \
        }, __FILE__, __LINE__, #expression)).CR_ASSERT_IMPL_A

#endif
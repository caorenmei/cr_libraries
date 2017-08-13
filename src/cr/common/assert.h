#ifndef CR_COMMON_ASSERT_H_
#define CR_COMMON_ASSERT_H_

#include "assert_builder.h"
#include "assert_error.h"


#define CR_ASSERT_IMPL_A(x) CR_ASSERT_IMPL_OP(x, B)
#define CR_ASSERT_IMPL_B(x) CR_ASSERT_IMPL_OP(x, A)
#define CR_ASSERT_IMPL_OP(x, next) CR_ASSERT_IMPL_A.print((x), #x).CR_ASSERT_IMPL_##next

// 安全的断言，断言失败抛出异常.
#define CR_ASSERT_E(E, expression)\
    if (!(expression))\
        throw E("") = cr::AssertBuilder<E>(#expression, __FILE__, __LINE__).CR_ASSERT_IMPL_A

// 安全的断言，断言失败抛出cr::AssertError异常.
#define CR_ASSERT(expression) CR_ASSERT_E(cr::AssertError, expression)


#endif
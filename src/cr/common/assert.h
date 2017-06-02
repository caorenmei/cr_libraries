#ifndef CR_COMMON_ASSERT_H_
#define CR_COMMON_ASSERT_H_

#include <cr/common/assert_builder.h>
#include <cr/common/assert_error.h>
#include <cr/common/throw.h>

#define cr_assert_impl_a(x) cr_assert_impl_op(x, b)
#define cr_assert_impl_b(x) cr_assert_impl_op(x, a)
#define cr_assert_impl_op(x, next) print((x), #x).cr_assert_impl_##next

/**
 * 安全的断言，断言失败抛出AssertError异常.
 *
 * @see cr::AssertError
 * @param 断言表达式.
 */
#define cr_assert(expression)\
	if (!(expression))\
		cr::AssertBuilder::sAssertCount = (cr::AssertBuilder(\
		[](const char* message)\
		{ \
			cr_throw(cr::AssertError, message); \
		}, __FILE__, __LINE__, #expression)).cr_assert_impl_a

#endif
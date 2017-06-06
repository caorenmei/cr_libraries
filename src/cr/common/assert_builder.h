#ifndef CR_COMMON_ASSERT_BUILDER_H_
#define CR_COMMON_ASSERT_BUILDER_H_

#include <functional>
#include <sstream>

namespace cr
{
	class AssertBuilder;

	/** 抛出断言. */
	class ThrowAssertError
	{
	public:
		explicit ThrowAssertError(const AssertBuilder& builder);

		operator int();

	private:
		const AssertBuilder& builder_;
	};

	/** 断言构造器 */
	class AssertBuilder
	{
	public:

		/** 断言发生次数 */
		static int sAssertCount;

		ThrowAssertError CR_ASSERT_IMPL_A;
		ThrowAssertError CR_ASSERT_IMPL_B;

		/**
		 * Constructor.
		 *
		 * @param handler 断言异常抛出.
		 * @param file 断言发生文件名.
		 * @param line 断言发生行数.
		 * @param expression 断言表达式.
		 */
		AssertBuilder(std::function<void(const char*)> handler, const char* file, int line, const char* expression);

		/**
		 * 打印变量名.
		 *
		 * @tparam T 变量类型.
		 * @param x 变量.
		 * @param varName 变量名.
		 *
		 * @return 断言构造器.
		 */
		template <typename T>
		AssertBuilder& print(const T& x, const char* varName);

	private:

		friend ThrowAssertError;

		// 断言描述
		std::stringstream message_;
		// 异常抛出
		std::function<void(const char*)> handler_;
	};

	template <typename T>
	AssertBuilder& AssertBuilder::print(const T& x, const char* varName)
	{
		message_ << "\t" << varName << " = " << x << "\n";
		return *this;
	}
}

#endif
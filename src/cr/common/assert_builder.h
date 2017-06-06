#ifndef CR_COMMON_ASSERT_BUILDER_H_
#define CR_COMMON_ASSERT_BUILDER_H_

#include <functional>
#include <sstream>

namespace cr
{
	class AssertBuilder;

	/** �׳�����. */
	class ThrowAssertError
	{
	public:
		explicit ThrowAssertError(const AssertBuilder& builder);

		operator int();

	private:
		const AssertBuilder& builder_;
	};

	/** ���Թ����� */
	class AssertBuilder
	{
	public:

		/** ���Է������� */
		static int sAssertCount;

		ThrowAssertError CR_ASSERT_IMPL_A;
		ThrowAssertError CR_ASSERT_IMPL_B;

		/**
		 * Constructor.
		 *
		 * @param handler �����쳣�׳�.
		 * @param file ���Է����ļ���.
		 * @param line ���Է�������.
		 * @param expression ���Ա��ʽ.
		 */
		AssertBuilder(std::function<void(const char*)> handler, const char* file, int line, const char* expression);

		/**
		 * ��ӡ������.
		 *
		 * @tparam T ��������.
		 * @param x ����.
		 * @param varName ������.
		 *
		 * @return ���Թ�����.
		 */
		template <typename T>
		AssertBuilder& print(const T& x, const char* varName);

	private:

		friend ThrowAssertError;

		// ��������
		std::stringstream message_;
		// �쳣�׳�
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
#ifndef CR_COMMON_ASSERT_ERROR_H_
#define CR_COMMON_ASSERT_ERROR_H_

#include <string>

#include <cr/common/error.h>

namespace cr
{

	/** ���Դ��� */
	class AssertError : public Error
	{
	public:
		using Error::Error;
	};
}

#endif

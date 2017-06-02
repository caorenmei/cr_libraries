#ifndef CR_COMMON_ASSERT_ERROR_H_
#define CR_COMMON_ASSERT_ERROR_H_

#include <string>

#include <cr/common/error.h>

namespace cr
{

	/** ¶ÏÑÔ´íÎó */
	class AssertError : public Error
	{
	public:
		using Error::Error;
	};
}

#endif

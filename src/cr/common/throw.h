#ifndef CR_COMMON_THROW_H_
#define CR_COMMON_THROW_H_

#include <cr/common/error.h>
#include <cr/common/exception.h>

/**
 * 该宏用于抛出cr::Error, cr::Exception及其子类，并自动设置行号与源文件.
 *
 * @see cr:Error
 * @see cr::Exception
 * @param E cr::Error, cr::Exception及其子类
 * @param ... 构造函数参数.
 * @exception E
 */
#define cr_throw(E, ...) \
	do \
	{ \
		E e(__VA_ARGS__); \
		e.setSourceName(__FILE__); \
		e.setSourceLine(__LINE__); \
		throw e; \
	} while(0)

#endif
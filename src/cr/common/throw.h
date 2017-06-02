#ifndef CR_COMMON_THROW_H_
#define CR_COMMON_THROW_H_

#include <cr/common/error.h>
#include <cr/common/exception.h>

/**
 * �ú������׳�cr::Error, cr::Exception�������࣬���Զ������к���Դ�ļ�.
 *
 * @see cr:Error
 * @see cr::Exception
 * @param E cr::Error, cr::Exception��������
 * @param ... ���캯������.
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
#ifndef CR_COMMON_EXCEPTION_H_
#define CR_COMMON_EXCEPTION_H_

#include <exception>

namespace cr
{
	/**
	* 异常基类，描述发生了异常
	*/
	class Exception : public std::exception
	{
	public:

		/**
		* Constructor.
		*
		* @param	异常描述信息.
		*/
		explicit Exception(const char* message = "");

		/**
		* Copy constructor.
		*
		* @param	other	The other.
		*/
		Exception(const Exception& other);

		/** Destructor. */
		virtual ~Exception();

		/**
		* Assignment operator.
		*
		* @param	other	The other.
		*
		* @return	A shallow copy of this object.
		*/
		Exception& operator=(const Exception& other);

		/**
		* 获取异常描述信息
		*
		* @return	异常描述信息
		*/
		virtual const char* what() const override;

		/**
		* 设置异常发生的源文件名
		*
		* @param 源文件名
		*/
		void setSourceName(const char* sourceName);

		/**
		* 获取异常发生的源文件名
		*
		* @return 异常发生的源文件名
		*/
		const char* getSourceName() const;

		/**
		* 设置异常发生的源码行
		*
		* @param 异常发生的源码行
		*/
		void setSourceLine(int sourceLine);

		/**
		* 获取异常发生的源码行
		*
		* @return 异常发生的源码行
		*/
		int getSourceLine() const;

	private:

		// 异常描述信息
		char* message_;
		// 源文件名
		const char* sourceName_;
		// 源码行
		int sourceLine_;
	};
}

#endif
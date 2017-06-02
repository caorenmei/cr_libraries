#ifndef CR_COMMON_EXCEPTION_H_
#define CR_COMMON_EXCEPTION_H_

#include <exception>

namespace cr
{
	/**
	* �쳣���࣬�����������쳣
	*/
	class Exception : public std::exception
	{
	public:

		/**
		* Constructor.
		*
		* @param	�쳣������Ϣ.
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
		* ��ȡ�쳣������Ϣ
		*
		* @return	�쳣������Ϣ
		*/
		virtual const char* what() const override;

		/**
		* �����쳣������Դ�ļ���
		*
		* @param Դ�ļ���
		*/
		void setSourceName(const char* sourceName);

		/**
		* ��ȡ�쳣������Դ�ļ���
		*
		* @return �쳣������Դ�ļ���
		*/
		const char* getSourceName() const;

		/**
		* �����쳣������Դ����
		*
		* @param �쳣������Դ����
		*/
		void setSourceLine(int sourceLine);

		/**
		* ��ȡ�쳣������Դ����
		*
		* @return �쳣������Դ����
		*/
		int getSourceLine() const;

	private:

		// �쳣������Ϣ
		char* message_;
		// Դ�ļ���
		const char* sourceName_;
		// Դ����
		int sourceLine_;
	};
}

#endif
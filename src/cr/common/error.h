#ifndef CR_COMMON_ERROR_H_
#define CR_COMMON_ERROR_H_

namespace cr
{
	/**
	 * ������࣬���ڲ��ɻָ��Ĵ���
	 */
	class Error
	{
	public:

		/**
		 * Constructor.
		 *
		 * @param	����������Ϣ.
		 */
		explicit Error(const char* message = "");

		/**
		 * Copy constructor.
		 *
		 * @param	other	The other.
		 */
		Error(const Error& other);

		/** Destructor. */
		virtual ~Error() noexcept;

		/**
		 * Assignment operator.
		 *
		 * @param	other	The other.
		 *
		 * @return	A shallow copy of this object.
		 */
		Error& operator=(const Error& other);

		/**
		 * ��ȡ����������Ϣ
		 *
		 * @return	����������Ϣ
		 */
		virtual const char* what() const;

		/**
		 * ���ô�������Դ�ļ���
		 *
		 * @param Դ�ļ���
		 */
		void setSourceName(const char* sourceName);

		/**
		 * ��ȡ��������Դ�ļ���
		 *
		 * @return ��������Դ�ļ���
		 */
		const char* getSourceName() const;

		/**
		 * ���ô�������Դ����
		 *
		 * @param ��������Դ����
		 */
		void setSourceLine(int sourceLine);

		/**
		* ��ȡ��������Դ����
		*
		* @return ��������Դ����
		*/
		int getSourceLine() const;

	private:

		// ����������Ϣ
		char* message_;
		// Դ�ļ���
		const char* sourceName_;
		// Դ����
		int sourceLine_;
	};
}

#endif

#ifndef CR_COMMON_ERROR_H_
#define CR_COMMON_ERROR_H_

#include <string>

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
         * @param    ����������Ϣ.
         */
        explicit Error(std::string message = "");

        /** Destructor. */
        virtual ~Error() noexcept;

        /**
         * ��ȡ����������Ϣ
         *
         * @return    ����������Ϣ
         */
        virtual const char* what() const;
       
        /**
         * ��ȡ����������Ϣ
         *
         * @return    ����������Ϣ
         */
        virtual const std::string& getMessage() const;

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
        std::string message_;
        // Դ�ļ���
        const char* sourceName_;
        // Դ����
        int sourceLine_;
    };
}

#endif

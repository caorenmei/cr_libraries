#ifndef CR_COMMON_ERROR_H_
#define CR_COMMON_ERROR_H_

#include <string>

namespace cr
{
    /**
     * 错误基类，用于不可恢复的错误
     */
    class Error
    {
    public:

        /**
         * Constructor.
         *
         * @param    错误描述信息.
         */
        explicit Error(std::string message = "");

        /** Destructor. */
        virtual ~Error() noexcept;

        /**
         * 获取错误描述信息
         *
         * @return    错误描述信息
         */
        virtual const char* what() const;
       
        /**
         * 获取错误描述信息
         *
         * @return    错误描述信息
         */
        virtual const std::string& getMessage() const;

        /**
         * 设置错误发生的源文件名
         *
         * @param 源文件名
         */
        void setSourceName(const char* sourceName);

        /**
         * 获取错误发生的源文件名
         *
         * @return 错误发生的源文件名
         */
        const char* getSourceName() const;

        /**
         * 设置错误发生的源码行
         *
         * @param 错误发生的源码行
         */
        void setSourceLine(int sourceLine);

        /**
        * 获取错误发生的源码行
        *
        * @return 错误发生的源码行
        */
        int getSourceLine() const;

    private:

        // 错误描述信息
        std::string message_;
        // 源文件名
        const char* sourceName_;
        // 源码行
        int sourceLine_;
    };
}

#endif

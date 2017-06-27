#ifndef CR_RAFT_EXCEPTION_H_
#define CR_RAFT_EXCEPTION_H_

#include <cr/common/exception.h>

namespace cr
{
    namespace raft
    {
        /** Raft异常 */
        class RaftException : public cr::Exception
        {
        public:
            using cr::Exception::Exception;
        };

        /** 参数异常 */
        class ArgumentException : public RaftException
        {
        public:
            using RaftException::RaftException;
        };

        /** 日志存储异常 */
        class StoreException : public RaftException
        {
        public:

            using RaftException::RaftException;

            /**
             * 构造函数
             * @param message 错误消息
             * @param errorCode 错误码
             */
            StoreException(std::string message, int errorCode);

            /**
             * 获取错误码
             * @param 错误码
             */
            int getErrorCode() const;

        private:

            int errorCode_;
        };
    }
}

#endif

#ifndef CR_RAFT_EXCEPTION_H_
#define CR_RAFT_EXCEPTION_H_

#include <cr/common/exception.h>

namespace cr
{
    namespace raft
    {
        namespace error
        {
            /** raft 涉及的错误码 */
            enum Errors : int
            {
                /** 操作成功 */
                SUCCESS,
                /** 不支持的操作 */
                NOT_SUPPORT,
                /** IO异常 */
                IO_ERROR,
                /** 序号错误 */
                LOG_INDEX_ERROR,
                /** 日志数据错误 */
                LOG_DATA_ERROR,
                /** 没有该日志  */
                NO_LOG_INDEX,
                /** 实例没有日志  */
                NO_LOG_DATA,
                /** 没有快照 */
                NO_CHECKPOINT,
                /** 日志已生成快照 */
                IN_CHECKPOINT,
            };
        }

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

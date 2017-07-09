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

        /** 参数错误 */
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
        };
    }
}

#endif

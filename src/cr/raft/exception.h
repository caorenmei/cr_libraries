#ifndef CR_RAFT_EXCEPTION_H_
#define CR_RAFT_EXCEPTION_H_

#include <cr/common/exception.h>

namespace cr
{
    namespace raft
    {
        /** 构造异常 */
        class ArgumentException : public cr::Exception
        {
        public:
            using cr::Exception::Exception;
        };
    }
}

#endif

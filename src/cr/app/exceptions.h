#ifndef CR_APP_EXCEPTIONS_H_
#define CR_APP_EXCEPTIONS_H_

#include <cr/common/exception.h>

namespace cr
{
    namespace app
    {
        // 服务异常
        class ServiceException : public Exception
        {
        public:
            using Exception::Exception;
        };
    }
}

#endif

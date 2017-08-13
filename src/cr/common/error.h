#ifndef CR_COMMON_ERROR_H_
#define CR_COMMON_ERROR_H_

#include "exception.h"

namespace cr
{
    /**
     * 错误基类，用于不可恢复的错误
     */
    class Error : public Exception
    {
    public:
        using Exception::Exception;
        
    };
}

#endif

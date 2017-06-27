#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {
        StoreException::StoreException(std::string message, int errorCode)
            : RaftException(std::move(message)),
            errorCode_(errorCode)
        {}

        int StoreException::getErrorCode() const
        {
            return errorCode_;
        }
    }
}
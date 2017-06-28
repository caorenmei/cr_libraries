#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {
        RaftException::RaftException(std::string message, int errorCode)
            : Exception(std::move(message)),
            errorCode_(errorCode)
        {}

        int RaftException::getErrorCode() const
        {
            return errorCode_;
        }
    }
}
#include "any.h"

namespace cr
{
    namespace protobuf
    {

        std::string getAnyTypeName(const google::protobuf::Any& message)
        {
            const std::string& url = message.type_url();
            auto pos = url.find_last_of('/');
            if (pos != std::string::npos)
            {
                pos = pos + 1;
                return url.substr(pos, url.size() - pos);
            }
            return {};
        }

    }
}
#include "any.h"

#include <google/protobuf/any.pb.h>

namespace cr
{
    namespace protobuf
    {

        std::string getAnyTypeName(const std::string& typeName)
        {
            return google::protobuf::internal::kTypeGoogleApisComPrefix + typeName;
        }

    }
}
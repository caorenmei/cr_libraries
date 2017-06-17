#ifndef CR_NETWORK_PROTOBUF_UTILS_H_
#define CR_NETWORK_PROTOBUF_UTILS_H_

#include <google/protobuf/message.h>

namespace cr
{
    namespace network
    {
        google::protobuf::Message* getProtobufMessageFromName(const std::string& typeName);
    }
}

#endif

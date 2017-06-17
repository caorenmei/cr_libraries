#ifndef CR_NETWORK_PROTOBUF_UTILS_H_
#define CR_NETWORK_PROTOBUF_UTILS_H_

#include <google/protobuf/message.h>

namespace cr
{
    namespace network
    {
        /**
         * 通过消息名字动态构造消息
         * @param typeName 消息名字
         * @return protobuf消息，nullptr 构造失败
         */
        google::protobuf::Message* getProtobufMessageFromName(const std::string& typeName);
    }
}

#endif

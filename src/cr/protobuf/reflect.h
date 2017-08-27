#ifndef CR_PROTOBUF_REFLECT_H_
#define CR_PROTOBUF_REFLECT_H_

#include <google/protobuf/message.h>

namespace cr
{
    namespace protobuf
    {
        /**
         * 通过消息名字动态构造消息
         * @param typeName 消息名字
         * @return protobuf消息，nullptr 构造失败
         */
        google::protobuf::Message* getMessageFromName(const std::string& typeName);

    }
}

#endif
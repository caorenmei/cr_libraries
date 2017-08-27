#ifndef CR_PROTOBUF_ANY_H_
#define CR_PROTOBUF_ANY_H_

#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>

namespace cr
{
    namespace protobuf
    {
        /**
         * 获取any的消息名字
         * @param message any 消息
         * @return 消息名字
         */
        std::string getAnyTypeName(const google::protobuf::Any& message);

    }
}

#endif

#ifndef CR_NETWORK_PROTOBUF_UTILS_H_
#define CR_NETWORK_PROTOBUF_UTILS_H_

#include <boost/asio/buffer.hpp>
#include <google/protobuf/message.h>

#include <cr/network/byte_buffer.h>

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


        /**
         * 从缓冲区解析protobuf消息
         * @param message protbuf 消息
         * @param b 缓冲区
         * @return True成功，False失败
         */
        bool parseProtobufMessage(google::protobuf::Message& message, const ByteBuffer& b);

        /**
         * 从缓冲区解析protobuf消息
         * @param message protbuf 消息
         * @param b 缓冲区
         * @return True成功，False失败
         */
        bool parseProtobufMessage(google::protobuf::Message& message, const ByteBuffer::ConstBuffers& b);


        /**
         * 从缓冲区解析protobuf消息
         * @param message protbuf 消息
         * @param b 缓冲区
         * @return True成功，False失败
         */
        bool parseProtobufMessage(google::protobuf::Message& message, const ByteBuffer::MutableBuffers& b);

        /**
         * 序列化protobuf消息到缓冲区
         * @param message protbuf 消息
         * @param b 缓冲区
         * @return True成功，False失败
         */
        bool serializeProtobufMessage(const google::protobuf::Message& message, ByteBuffer& b);
    }
}

#endif
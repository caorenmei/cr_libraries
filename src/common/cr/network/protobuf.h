#ifndef CR_COMMON_NETOWRK_PROTOBUF_H_
#define CR_COMMON_NETOWRK_PROTOBUF_H_

#include <functional>
#include <memory>

#include <google/protobuf/message.h>

#include "byte_buffer.h"

namespace cr
{
    namespace network
    {

        /** tcp连接 */
        class Connection;

        /** Protobuf 消息编解码器 */
        class ProtobufCodec
        {
        public:

            /** 错误码 */
            enum ErrorCode
            {
                /** 没有错误 */
                ERROR_NONE = 0,
                /** 消息长度错误 */
                ERROR_LENGTH = 1,
                /** 检验码错误 */
                ERROR_CHECKSUM = 2,
                /** 未知协议 */
                ERROR_UNKONW_PROTO = 3,
                /** 解析错误 */
                ERROR_PARSE = 4,
            };

            /** 消息回调 */
            using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, const std::shared_ptr<google::protobuf::Message>&)>;

            /** 错误回调 */
            using ErrorCallback = std::function<void(const std::shared_ptr<Connection>&, ErrorCode)>;

            /**
             * 发送消息
             * @param conn 连接
             * @param message 消息
             */
            void send(const std::shared_ptr<Connection>& conn, const google::protobuf::Message& message);

            /**
             * 消息回调
             * @param conn 连接
             * @param buffer 缓冲区
             */
            void onMessage(const std::shared_ptr<Connection>& conn, ByteBuffer& buffer);

            /**
             * 设置消息回调
             * @param cb 回调
             */
            void setMessageCallback(MessageCallback cb);

            /**
             * 设置错误回调
             * @param cb 错误回调
             */
            void setErrorCallback(ErrorCallback cb);

        private:

            // 消息回调
            MessageCallback messageCallback_;
        };
    }
}

#endif

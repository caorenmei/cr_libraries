#ifndef CR_COMMON_NETOWRK_PROTOBUF_CODEC_H_
#define CR_COMMON_NETOWRK_PROTOBUF_CODEC_H_

#include <cstdint>
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
                /** 消息类型错误 */
                ERROR_TYPE = 3,
                /** 解析错误 */
                ERROR_PARSE = 4,
            };

            /** 消息回调 */
            using MessageCallback = std::function<void(const std::shared_ptr<Connection>&, const std::shared_ptr<google::protobuf::Message>&)>;

            /** 错误回调 */
            using ErrorCallback = std::function<void(const std::shared_ptr<Connection>&, int)>;

            /**
             * 构造函数
             * @param checksum 验证校验码
             * @param maxPacketLength 最大包长
             */
            explicit ProtobufCodec(bool verifyCheck = true, std::uint32_t maxPacketLength = 64 * 1024 * 1024);

            /** 析构函数 */
            ~ProtobufCodec();

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

            // 使用校验码
            bool verifyCheck_;
            // 最大包长
            std::size_t maxPacketLength_;
            // 消息回调
            MessageCallback messageCallback_;
            // 错误回调
            ErrorCallback errorCallback_;
            // 缓冲区
            std::string tempBuffer_;
        };
    }
}

#endif

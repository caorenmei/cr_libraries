#ifndef CR_NETWORK_CONNECTION_H_
#define CR_NETWORK_CONNECTION_H_

#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <google/protobuf/message.h>

#include <cr/common/logging.h>

#include "byte_buffer.h"

namespace cr
{
    namespace network
    {
        /** socket连接类 */
        class PbConnection : public std::enable_shared_from_this<PbConnection>
        {
        public:

            /** 消息处理回调 */
            using MessageHandler = std::function<void(const std::shared_ptr<PbConnection>&, std::shared_ptr<google::protobuf::Message>)>;

            /** 连接回调 */
            using CloseHandler = std::function<void(const std::shared_ptr<PbConnection>&)>;

            /** 连接参数 */
            struct Options
            {
                /** 最大包长度 */
                std::uint32_t maxPacketLength = 16 * 1024 * 1024;
                /** 忽略校验码 */
                bool verifyChecksum = true;
            };

            /**
             * 构造函数
             * @param socket socket
             * @param logger 日志
             * @param options 连接参数
             */
            PbConnection(boost::asio::ip::tcp::socket socket, cr::log::Logger& logger, const Options& options = Options());

            /** 析构函数 */
            ~PbConnection();

            PbConnection(const PbConnection&) = delete;
            PbConnection& operator=(const PbConnection&) = delete;

            /**
             * 设置消息处理器 
             * @param handler 消息处理器
             */
            void setMessageHandler(MessageHandler handler);

            /**
             * 设置连接断开回调
             * @param handler 连接断开回调
             */
            void setCloseHandler(CloseHandler handler);

            /**
             * 发送消息
             * @param message 消息
             */
            void send(std::shared_ptr<google::protobuf::Message> message);

            /**
             * 开始读取数据
             */
            void start();

            /**
             * 关闭socket
             */
            void close();

        private:

            // 缓冲区序列
            using MutableBuffers = cr::network::ByteBuffer::MutableBuffers;

            // 开始读
            void startRead();

            // 输入回调
            void onReadHandler(std::size_t length);

            //开始写
            void startWrite();

            // 输出回调
            void onWriteHandler(std::size_t length);

            // 错误处理器
            void onErrorHandler(const boost::system::error_code& error);

            // 解码输入
            bool decodePacket();

            // 获取缓冲区的名字
            bool getMessageTypeName(const cr::network::ByteBuffer::MutableBuffers& data, std::string& name);

            // 计算校验码
            bool checkChecksum(const cr::network::ByteBuffer::MutableBuffers& data, std::uint32_t checksum);

            // 反序列化pb
            bool parseMessage(const std::string& name, const MutableBuffers& data, std::shared_ptr<google::protobuf::Message>& message);

            // 解析成任意消息
            std::shared_ptr<google::protobuf::Message> parseAnyMessage(const std::string& name, const MutableBuffers& data);

            // 编码消息包
            void encodePacket(const std::shared_ptr<google::protobuf::Message>& message);

            // 获取消息名字
            std::string getMessageTypeName(const std::shared_ptr<google::protobuf::Message>& message);

            // 序列化消息
            void serializeMessage(const std::shared_ptr<google::protobuf::Message>& message, ByteBuffer& buffer);

            // 计算校验码
            std::uint16_t calcChecksum(const MutableBuffers& data);

            // socket 
            boost::asio::ip::tcp::socket socket_;
            // 日志
            cr::log::Logger& logger_;
            // 参数
            Options options_;
            // 状态
            enum State { NORMAL, CONNECTED, DISCONNECTING, DISCONNECTED };
            State state_;
            // 输入缓冲区
            cr::network::ByteBuffer inputBuffer_;
            // 输出缓冲区
            cr::network::ByteBuffer firstOutputBuffer_;
            cr::network::ByteBuffer secondOutputBuffer_;
            // 消息处理器
            MessageHandler messageHandler_;
            // 关闭处理器
            CloseHandler closeHandler_;
        };
    }
}

#endif

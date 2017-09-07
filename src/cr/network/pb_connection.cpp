#include "pb_connection.h"

#include <algorithm>

#include <boost/crc.hpp>
#include <boost/endian/arithmetic.hpp>
#include <google/protobuf/any.pb.h>

#include <cr/protobuf/any.h>
#include <cr/protobuf/reflect.h>

#include "pb_serialize.h"

namespace cr
{
    namespace network
    {

        PbConnection::PbConnection(boost::asio::ip::tcp::socket socket, cr::log::Logger& logger, const Options& options/* = Options()*/)
            : socket_(std::move(socket)),
            logger_(logger),
            options_(options)
        {}

        PbConnection::~PbConnection()
        {}

        void PbConnection::setMessageHandler(MessageHandler handler)
        {
            messageHandler_ = std::move(handler);
        }

        void PbConnection::setCloseHandler(CloseHandler handler)
        {
            closeHandler_ = std::move(handler);
        }

        void PbConnection::send(std::shared_ptr<google::protobuf::Message> message)
        {
            if (state_ == CONNECTED)
            {
                encodePacket(message);
                if (firstOutputBuffer_.size() == 0)
                {
                    firstOutputBuffer_.swap(secondOutputBuffer_);
                    startWrite();
                }
            }
        }

        void PbConnection::start()
        {
            if (state_ == NORMAL)
            {
                state_ = CONNECTED;
                startRead();
            }
        }

        void PbConnection::close()
        {
            if (state_ == CONNECTED)
            {
                state_ = DISCONNECTING;
                socket_.close();
            }
        }

        void PbConnection::startRead()
        {
            socket_.async_receive(inputBuffer_.prepare(4 * 1024), 
                [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t length)
            {
                if (!error)
                {
                    onReadHandler(length);
                }
                else
                {
                    CRLOG_TRACE(logger_, "PbConnection") << "Handle Socket Error: Code=" << error.value() << ", Message=" << error.message();
                    onErrorHandler(error);
                }
            });
        }

        void PbConnection::onReadHandler(std::size_t length)
        {
            if (state_ == CONNECTED)
            {
                inputBuffer_.commit(length);
                if (decodePacket())
                {
                    startRead();
                }
            }
        }

        void PbConnection::startWrite()
        {
            socket_.async_write_some(firstOutputBuffer_.data(), [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t length)
            {
                if (!error)
                {
                    onWriteHandler(length);
                }
                else
                {
                    CRLOG_TRACE(logger_, "PbConnection") << "Handle Socket Error: Code=" << error.value() << ", Message=" << error.message();
                    onErrorHandler(error);
                }
            });
        }

        void PbConnection::onWriteHandler(std::size_t length)
        {
            firstOutputBuffer_.consume(length);
            if (firstOutputBuffer_.size() == 0)
            {
                firstOutputBuffer_.swap(secondOutputBuffer_);
            }
            if (firstOutputBuffer_.size() != 0)
            {
                startWrite();
            }
        }

        void PbConnection::onErrorHandler(const boost::system::error_code& error)
        {
            if (state_ != DISCONNECTED)
            {
                state_ = DISCONNECTED;
                if (closeHandler_)
                {
                    closeHandler_(shared_from_this());
                }
                messageHandler_ = nullptr;
                closeHandler_ = nullptr;
            }
        }

        bool PbConnection::decodePacket()
        {
            std::uint32_t packetLength;
            std::uint16_t checksum;
            // 循环解码,| 4 length | name + '\0'| 2 checksum |
            while (inputBuffer_.size() >= sizeof(packetLength) + sizeof(checksum) + 1)
            {
                // 包长
                boost::asio::buffer_copy(boost::asio::buffer(&packetLength, sizeof(packetLength)), inputBuffer_.data(0, sizeof(packetLength)));
                boost::endian::little_to_native_inplace(packetLength);
                // 包长非法
                if (packetLength > options_.maxPacketLength)
                {
                    CRLOG_WARN(logger_, "PbConnection") << "Packet Length To Long: Length=" << packetLength << ", MaxLength=" << options_.maxPacketLength;
                    close();
                    return false;
                }
                // 包长不够
                if (inputBuffer_.size() < packetLength)
                {
                    return true;
                }
                // 名字
                std::size_t typeNameIndex = sizeof(packetLength);
                std::size_t maxTypeNameLength = packetLength - typeNameIndex - sizeof(checksum);
                std::string typeName;
                if (!getMessageTypeName(inputBuffer_.data(typeNameIndex, maxTypeNameLength), typeName))
                {
                    CRLOG_WARN(logger_, "PbConnection") << "Packet Message Name Bad";
                    close();
                    return false;
                }
                // 校验码
                std::size_t checksumIndex = packetLength - sizeof(checksum);
                boost::asio::buffer_copy(boost::asio::buffer(&checksum, sizeof(checksum)), inputBuffer_.data(checksumIndex, sizeof(checksum)));
                boost::endian::little_to_native_inplace(checksum);
                // 比较校验码
                if (options_.verifyChecksum && !checkChecksum(inputBuffer_.data(0, checksumIndex), checksum))
                {
                    CRLOG_WARN(logger_, "PbConnection") << "Check Checksum Failed: Checksum=" << checksum;
                    close();
                    return false;
                }
                // 消息
                std::size_t dataIndex = sizeof(packetLength) + typeName.size() +1;
                std::size_t dataLength = packetLength - dataIndex - sizeof(checksum);
                std::shared_ptr<google::protobuf::Message> message;
                if (!parseMessage(typeName, inputBuffer_.data(dataIndex, dataLength), message))
                {
                    CRLOG_WARN(logger_, "PbConnection") << "Parse Message Failed: TypeName=" << typeName;
                    close();
                    return false;
                }
                // 处理消息
                if (messageHandler_)
                {
                    messageHandler_(shared_from_this(), message);
                }
            }
            return true;
        }

        bool PbConnection::getMessageTypeName(const cr::network::ByteBuffer::MutableBuffers& data, std::string& name)
        {
            for (std::size_t i = 0; i != data.size(); ++i)
            {
                const char* buffer = boost::asio::buffer_cast<const char*>(data[i]);
                std::size_t bufferSize = boost::asio::buffer_size(data[i]);
                for (std::size_t i = 0; i != bufferSize; ++i)
                {
                    if (buffer[i] != '\0')
                    {
                        name.push_back(buffer[i]);
                    }
                    else
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool PbConnection::checkChecksum(const cr::network::ByteBuffer::MutableBuffers& data, std::uint32_t checksum)
        {
            return calcChecksum(data) == checksum;
        }

        bool PbConnection::parseMessage(const std::string& name, const MutableBuffers& data, std::shared_ptr<google::protobuf::Message>& message)
        {
            // 消息
            message.reset(cr::protobuf::getMessageFromName(name));
            if (message != nullptr)
            {
                return cr::network::parseProtobufMessage(*message, data);
            }
            message = parseAnyMessage(name, data);
            return true;
        }

        std::shared_ptr<google::protobuf::Message> PbConnection::parseAnyMessage(const std::string& name, const MutableBuffers& data)
        {
            auto any = std::make_shared<google::protobuf::Any>();
            *any->mutable_type_url() = cr::protobuf::getAnyTypeName(name);
            auto& value = *any->mutable_value();
            value.resize(boost::asio::buffer_size(data));
            boost::asio::buffer_copy(boost::asio::buffer(&value[0], value.size()), data);
            return any;
        }

        void PbConnection::encodePacket(const std::shared_ptr<google::protobuf::Message>& message)
        {
            std::size_t originLength = secondOutputBuffer_.size();
            // 分配包长
            std::uint32_t packetLength;
            secondOutputBuffer_.prepare(sizeof(packetLength));
            secondOutputBuffer_.commit(sizeof(packetLength));
            // 写入名字
            std::string typeName = getMessageTypeName(message);
            boost::asio::buffer_copy(secondOutputBuffer_.prepare(typeName.size() + 1), boost::asio::buffer(typeName.c_str(), typeName.size() + 1));
            secondOutputBuffer_.commit(typeName.size() + 1);
            // 写入包体
            serializeMessage(message, secondOutputBuffer_);
            // 写入包长
            std::uint16_t checksum;
            packetLength = secondOutputBuffer_.size() - originLength + sizeof(checksum);
            boost::endian::native_to_little_inplace(packetLength);
            secondOutputBuffer_.set(originLength, &packetLength, sizeof(packetLength));
            // 写入校验码
            checksum = calcChecksum(secondOutputBuffer_.data(originLength, secondOutputBuffer_.size() - originLength));
            boost::endian::native_to_little_inplace(checksum);
            boost::asio::buffer_copy(secondOutputBuffer_.prepare(sizeof(checksum)), boost::asio::buffer(&checksum, sizeof(checksum)));
            secondOutputBuffer_.commit(sizeof(checksum));
        }

        std::string PbConnection::getMessageTypeName(const std::shared_ptr<google::protobuf::Message>& message)
        {
            std::string typeName;
            if (message->GetDescriptor() == google::protobuf::Any::descriptor())
            {
                const std::string& typeUrl = static_cast<google::protobuf::Any&>(*message).type_url();
                google::protobuf::internal::ParseAnyTypeUrl(typeUrl, &typeName);
            }
            else
            {
                typeName = message->GetTypeName();
            }
            return typeName;
        }

        void PbConnection::serializeMessage(const std::shared_ptr<google::protobuf::Message>& message, ByteBuffer& buffer)
        {
            if (message->GetDescriptor() == google::protobuf::Any::descriptor())
            {
                const std::string& body = static_cast<google::protobuf::Any&>(*message).value();
                boost::asio::buffer_copy(buffer.prepare(body.size()), boost::asio::buffer(body));
                buffer.commit(body.size());
            }
            else
            {
                serializeProtobufMessage(*message, buffer);
            }
        }

        std::uint16_t PbConnection::calcChecksum(const MutableBuffers& data)
        {
            boost::crc_16_type crc16;
            for (std::size_t i = 0; i != data.size(); ++i)
            {
                crc16.process_bytes(boost::asio::buffer_cast<const void*>(data[i]), boost::asio::buffer_size(data[i]));
            }
            return crc16.checksum();
        }
    }
}
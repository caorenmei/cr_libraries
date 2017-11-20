#include "protobuf_codec.h"

#include <boost/crc.hpp>
#include <boost/endian/arithmetic.hpp>
#include <google/protobuf/any.h>
#include <google/protobuf/any.pb.h>

#include "byte_buffer_stream.h"
#include "connection.h"

namespace cr
{
    namespace network
    {
        ProtobufCodec::ProtobufCodec(bool verifyCheck/* = true*/, std::size_t maxPacketLength/* = 64 * 1024 * 1024*/)
            : verifyCheck_(verifyCheck),
            maxPacketLength_(maxPacketLength)
        {}

        ProtobufCodec::~ProtobufCodec()
        {}

        void ProtobufCodec::send(const std::shared_ptr<Connection>& conn, const google::protobuf::Message& message)
        {
            tempBuffer_.clear();
            std::uint32_t length = 0;
            tempBuffer_.resize(sizeof(length));
            if (message.GetDescriptor() == google::protobuf::Any::descriptor())
            {
                auto& any = static_cast<const google::protobuf::Any&>(message);
                std::string typeName;
                google::protobuf::internal::ParseAnyTypeUrl(any.type_url(), &typeName);
                tempBuffer_.append(typeName.c_str(), typeName.size() + 1);
                tempBuffer_.append(any.value());
            }
            else
            {
                tempBuffer_.append(message.GetTypeName());
                tempBuffer_.push_back('\0');
                message.AppendToString(&tempBuffer_);
            }
            std::uint16_t checksum = 0;
            length = tempBuffer_.size() + sizeof(checksum);
            length = boost::endian::native_to_little(length);
            std::memcpy(&tempBuffer_[0], &length, sizeof(length));
            if (verifyCheck_)
            {
                boost::crc_16_type crc;
                crc.process_bytes(tempBuffer_.data(), tempBuffer_.size());
                checksum = crc.checksum();
                checksum = boost::endian::native_to_little(checksum);
            }
            tempBuffer_.append(reinterpret_cast<char*>(&checksum), sizeof(checksum));
            conn->send(tempBuffer_.data(), tempBuffer_.size());
        }

        void ProtobufCodec::onMessage(const std::shared_ptr<Connection>& conn, ByteBuffer& buffer)
        {
            std::uint32_t length = 0;
            std::uint16_t checksum = 0;
            constexpr std::uint32_t minPacketLength = sizeof(length) + sizeof(checksum) + 1;
            while (buffer.size() >= minPacketLength)
            {
                boost::asio::buffer_copy(boost::asio::buffer(&length, sizeof(length)), buffer.data(0, sizeof(length)));
                length = boost::endian::little_to_native(length);
                if (length > maxPacketLength_)
                {
                    errorCallback_(conn, ERROR_LENGTH);
                    break;
                }
                if (length < buffer.size())
                {
                    break;
                }
                if (verifyCheck_)
                {
                    boost::asio::buffer_copy(boost::asio::buffer(&checksum, sizeof(checksum)), buffer.data(length - sizeof(checksum), sizeof(checksum)));
                    checksum = boost::endian::little_to_native(checksum);
                    boost::crc_16_type crc;
                    for (auto& data : buffer.data(0, length - sizeof(checksum)))
                    {
                        crc.process_bytes(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data));
                    }
                    if (checksum != crc.checksum())
                    {
                        errorCallback_(conn, ERROR_CHECKSUM);
                        break;
                    }
                }
                std::string typeName;
                for (auto& data : buffer.data(0, length - (sizeof(length) - sizeof(checksum))))
                {
                    auto begin = boost::asio::buffer_cast<const char*>(data);
                    auto end = begin + boost::asio::buffer_size(data);
                    auto iter = std::find(begin, end, '\0');
                    typeName.append(begin, iter);
                    if (iter != end)
                    {
                        break;
                    }
                }
                if (typeName.size() == length - (sizeof(length) - sizeof(checksum)))
                {
                    errorCallback_(conn, ERROR_UNKONW_PROTO);
                    break;
                }
                std::shared_ptr<google::protobuf::Message> message;
                auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
                if (descriptor != nullptr)
                {
                    auto prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
                    if (prototype != nullptr)
                    {
                        message.reset(prototype->New());
                    }
                }
                auto bodyLength = length - (sizeof(length) + typeName.size() + 1 + sizeof(checksum));
                auto body = buffer.data(sizeof(length) + typeName.size() + 1, bodyLength);
                if (message != nullptr)
                {
                    
                    ByteBufferInputStream stream(body);
                    if (!message->ParseFromZeroCopyStream(&stream))
                    {
                        errorCallback_(conn, ERROR_PARSE);
                        break;
                    }
                }
                else
                {
                    auto any = std::make_shared<google::protobuf::Any>();
                    any->set_type_url(google::protobuf::internal::kTypeGoogleApisComPrefix + typeName);
                    for (auto& data : body)
                    {
                        any->mutable_value()->append(boost::asio::buffer_cast<const char*>(data), boost::asio::buffer_size(data));
                    }
                    message = any;
                }
                messageCallback_(conn, message);
            }
        }

        void ProtobufCodec::setMessageCallback(MessageCallback cb)
        {
            messageCallback_ = std::move(cb);
        }

        void ProtobufCodec::setErrorCallback(ErrorCallback cb)
        {
            errorCallback_ = std::move(cb);
        }
    }
}
#include "pb_serialize.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace cr
{
    namespace network
    {

        class ByteBufferInputStream : public google::protobuf::io::ZeroCopyInputStream
        {
        public:

            explicit ByteBufferInputStream(const ByteBuffer::ConstBuffers& buffers);

            bool Next(const void ** data, int * size) override;

            void BackUp(int count) override;

            bool Skip(int count) override;

            google::protobuf::int64 ByteCount() const override;

        private:
            
            ByteBuffer::ConstBuffers buffers_;
            ByteBuffer::ConstBuffers::iterator iter_;
            std::size_t index_;
            google::protobuf::int64 byteCount_;
        };

        class ByteBufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream
        {
        public:

            explicit ByteBufferOutputStream(ByteBuffer& buffer);

            bool Next(void ** data, int * size) override;

            void BackUp(int count) override;

            google::protobuf::int64 ByteCount() const override;

        private:

            ByteBuffer& buffer_;
            std::size_t size_;
            google::protobuf::int64 byteCount_;
        };

        bool parseProtobufMessage(google::protobuf::Message& message, const ByteBuffer::ConstBuffers& b)
        {
            ByteBufferInputStream stream(b);
            return message.ParseFromZeroCopyStream(&stream);
        }

        bool parseProtobufMessage(google::protobuf::Message& message, const ByteBuffer::MutableBuffers& b)
        {
            return parseProtobufMessage(message, ByteBuffer::ConstBuffers{ b.begin(), b.end() });
        }

        bool serializeProtobufMessage(const google::protobuf::Message& message, ByteBuffer& b)
        {
            ByteBufferOutputStream stream(b);
            return message.SerializeToZeroCopyStream(&stream);
        }

        ByteBufferInputStream::ByteBufferInputStream(const ByteBuffer::ConstBuffers& buffers)
            : buffers_(buffers),
            iter_(buffers_.begin()),
            index_(0),
            byteCount_(0)
        {}

        bool ByteBufferInputStream::Next(const void ** data, int * size)
        {
            if (iter_ != buffers_.end())
            {
                *data = boost::asio::buffer_cast<const char*>(*iter_) + index_;
                std::size_t nodeSize = boost::asio::buffer_size(*iter_);
                *size = static_cast<std::size_t>(nodeSize - index_);
                index_ += *size;
                byteCount_ += *size;
                if (index_ == nodeSize)
                {
                    index_ = 0;
                    ++iter_;
                }
                return true;
            }
            return false;
        }

        void ByteBufferInputStream::BackUp(int count)
        {
            byteCount_ -= index_;
        }

        bool ByteBufferInputStream::Skip(int count)
        {
            while (iter_ != buffers_.end() && count > 0)
            {
                std::size_t nodeSize = boost::asio::buffer_size(*iter_);
                std::size_t validSize = static_cast<std::size_t>(nodeSize - index_);
                if (static_cast<std::size_t>(count) < validSize)
                {
                    index_ += count;
                    count = 0;
                }
                else
                {
                    index_ = 0;
                    ++iter_;
                    count -= validSize;
                }
            }
            return count == 0;
        }

        google::protobuf::int64 ByteBufferInputStream::ByteCount() const
        {
            return byteCount_;
        }

        ByteBufferOutputStream::ByteBufferOutputStream(ByteBuffer& buffer)
            : buffer_(buffer),
            size_(0),
            byteCount_(0)
        {}

        bool ByteBufferOutputStream::Next(void ** data, int * size)
        {
            buffer_.commit(size_);
            byteCount_ += static_cast<int>(size_);
            auto buffers = buffer_.prepare(1024);
            *data = boost::asio::buffer_cast<void*>(*buffers.begin());
            size_ = boost::asio::buffer_size(*buffers.begin());
            *size = static_cast<int>(size_);
            return true;
        }

        void ByteBufferOutputStream::BackUp(int count)
        {
            size_ -= static_cast<std::size_t>(count);
            buffer_.commit(size_);
            byteCount_ += static_cast<int>(size_);
        }

        google::protobuf::int64 ByteBufferOutputStream::ByteCount() const
        {
            return byteCount_;
        }
    }
}
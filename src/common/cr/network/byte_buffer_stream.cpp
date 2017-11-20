#include "byte_buffer_stream.h"
namespace cr
{
    namespace network
    {

        ByteBufferInputStream::ByteBufferInputStream(const ByteBuffer::ConstBuffers& b)
            : buffers_(b),
            iter_(buffers_.begin()),
            index_(0),
            count_(0)
        {}

        ByteBufferInputStream::ByteBufferInputStream(const ByteBuffer::MutableBuffers& b)
            : ByteBufferInputStream(ByteBuffer::ConstBuffers(b.begin(), b.end()))
        {}

        ByteBufferInputStream::~ByteBufferInputStream()
        {}

        bool ByteBufferInputStream::Next(const void** data, int* size)
        {
            if (iter_ != buffers_.end())
            {
                *data = boost::asio::buffer_cast<const char*>(*iter_) + index_;
                *size = static_cast<int>(boost::asio::buffer_size(*iter_)) - index_;
                count_ = count_ + static_cast<std::size_t>(*size);
                ++iter_;
                index_ = 0;
                return true;
            }
            return false;
        }

        void ByteBufferInputStream::BackUp(int count)
        {
            count_ = count_ - count;
            index_ = index_ - count;
        }

        bool ByteBufferInputStream::Skip(int count)
        {
            while (count > 0 && iter_ != buffers_.end())
            {
                auto bufferSize = boost::asio::buffer_size(*iter_);
                auto remainSize = bufferSize - index_;
                if (index_ + count < bufferSize)
                {
                    count_ = count_ + count;
                    count = 0;
                    index_ = index_ + count;
                }
                else
                {
                    count_ = count_ + remainSize;
                    count_ = count_ - remainSize;
                    index_ = 0;
                    ++iter_;
                }
            }
            return iter_ != buffers_.end();
        }

        google::protobuf::int64 ByteBufferInputStream::ByteCount() const
        {
            return count_;
        }

        ByteBufferOutputStream::ByteBufferOutputStream(ByteBuffer& b)
            : buffer_(b),
            originSize_(b.size())
        {}

        ByteBufferOutputStream::~ByteBufferOutputStream()
        {}

        bool ByteBufferOutputStream::Next(void** data, int* size)
        {
            auto buffers = buffer_.prepare(4 * 1024);
            *data = boost::asio::buffer_cast<char*>(buffers.front());
            *size = static_cast<int>(boost::asio::buffer_size(buffers.front()));
            buffer_.commit(boost::asio::buffer_size(buffers.front()));
            return true;
        }

        void ByteBufferOutputStream::BackUp(int count)
        {
            buffer_.uncommit(count);
        }
        
        google::protobuf::int64 ByteBufferOutputStream::ByteCount() const
        {
            return buffer_.size() - originSize_;
        }
    }
}
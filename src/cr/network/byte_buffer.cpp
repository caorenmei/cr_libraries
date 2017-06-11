#include <cr/network/byte_buffer.h>

#include <cstring>

#include <cr/common/assert.h>

namespace cr
{
    namespace network
    {
        ByteBuffer::ByteBuffer(std::size_t initialCapacity)
            : buffer_(initialCapacity),
            readerIndex_(0),
            writerIndex_(0),
            size_(0)
        {}

        ByteBuffer::ByteBuffer(ByteBuffer&& other)
            : buffer_(std::move(other.buffer_)),
            readerIndex_(other.readerIndex_),
            writerIndex_(other.writerIndex_),
            size_(other.size_)
        {
            other.readerIndex_ = 0;
            other.writerIndex_ = 0;
            other.size_ = 0;
        }

        ByteBuffer::~ByteBuffer()
        {}

        ByteBuffer& ByteBuffer::operator=(ByteBuffer&& other)
        {
            ByteBuffer(std::move(other)).swap(*this);
            return *this;
        }

        std::size_t ByteBuffer::getCapacity() const
        {
            return buffer_.size();
        }

        std::size_t ByteBuffer::getReadableBytes() const
        {
            return size_;
        }

        ByteBuffer::ConstBuffers ByteBuffer::data() const
        {
            ConstBuffers buffers;
            std::size_t readerIndex = readerIndex_;
            if (writerIndex_ <= readerIndex && size_ != 0)
            {
                buffers.emplace_back(buffer_.data() + readerIndex, buffer_.size() - readerIndex);
                readerIndex = 0;
            }
            if (readerIndex < writerIndex_)
            {
                buffers.emplace_back(buffer_.data() + readerIndex, writerIndex_ - readerIndex);
            }
            return buffers;
        }

        ByteBuffer::MutableBuffers ByteBuffer::data()
        {
            MutableBuffers buffers;
            std::size_t readerIndex = readerIndex_;
            if (writerIndex_ <= readerIndex && size_ != 0)
            {
                buffers.emplace_back(buffer_.data() + readerIndex, buffer_.size() - readerIndex);
                readerIndex = 0;
            }
            if (readerIndex < writerIndex_)
            {
                buffers.emplace_back(buffer_.data() + readerIndex, writerIndex_ - readerIndex);
            }
            return buffers;
        }

        ByteBuffer::ConstBuffers ByteBuffer::data(std::size_t n) const
        {
            CR_ASSERT(n <= size_)(n)(size_)(readerIndex_)(writerIndex_);
            ConstBuffers buffers;
            std::size_t readerIndex = readerIndex_;
            if (writerIndex_ <= readerIndex && n != 0)
            {
                std::size_t nbytes = std::min(buffer_.size() - readerIndex, n);
                buffers.emplace_back(buffer_.data() + readerIndex, nbytes);
                readerIndex = (readerIndex + nbytes) % buffer_.size();
                n = n - nbytes;
            }
            if (readerIndex < writerIndex_ && n != 0)
            {
                std::size_t nbytes = std::min(writerIndex_ - readerIndex, n);
                buffers.emplace_back(buffer_.data() + readerIndex, nbytes);
            }
            return buffers;
        }

        ByteBuffer::MutableBuffers ByteBuffer::data(std::size_t n)
        {
            CR_ASSERT(n <= size_)(n)(size_)(readerIndex_)(writerIndex_);
            MutableBuffers buffers;
            std::size_t readerIndex = readerIndex_;
            if (writerIndex_ <= readerIndex && n != 0)
            {
                std::size_t nbytes = std::min(buffer_.size() - readerIndex, n);
                buffers.emplace_back(buffer_.data() + readerIndex, nbytes);
                readerIndex = (readerIndex + nbytes) % buffer_.size();
                n = n - nbytes;
            }
            if (readerIndex < writerIndex_ && n != 0)
            {
                std::size_t nbytes = std::min(writerIndex_ - readerIndex, n);
                buffers.emplace_back(buffer_.data() + readerIndex, nbytes);
            }
            return buffers;
        }

        void ByteBuffer::consume(std::size_t n)
        {
            CR_ASSERT(n <= size_)(n)(size_)(readerIndex_)(writerIndex_);
            readerIndex_ = (readerIndex_ + n) % buffer_.size();
            size_ = size_ - n;
        }

        ByteBuffer::MutableBuffers ByteBuffer::prepare(std::size_t n)
        {
            ensure(n);
            MutableBuffers buffers;
            std::size_t writerIndex = writerIndex_;
            if (readerIndex_ <= writerIndex && n != 0)
            {
                std::size_t nbytes = std::min(buffer_.size() - writerIndex, n);
                buffers.emplace_back(buffer_.data() + writerIndex, nbytes);
                writerIndex = (writerIndex + nbytes) % buffer_.size();
                n = n - nbytes;
            }
            if (writerIndex < readerIndex_ && n != 0)
            {
                std::size_t nbytes = std::min(readerIndex_ - writerIndex, n);
                buffers.emplace_back(buffer_.data() + writerIndex, nbytes);
            }
            return buffers;
        }

        void ByteBuffer::commit(std::size_t n)
        {
            CR_ASSERT(n <= buffer_.size() - size_)(n)(size_)(buffer_.size())(readerIndex_)(writerIndex_);
            writerIndex_ = (writerIndex_ + n) % buffer_.size();
            size_ = size_ + n;
        }

        void ByteBuffer::set(std::size_t offset, const void* source, std::size_t n)
        {
            CR_ASSERT(source != nullptr && offset + n <= size_)(source)(offset)(n)(size_);
            if (n != 0)
            {
                std::size_t readerIndex = (readerIndex_ + offset) % buffer_.size();
                std::size_t nbytes = std::min(buffer_.size() - readerIndex, n);
                std::memcpy(buffer_.data() + readerIndex, source, nbytes);
                source = static_cast<const char*>(source) + nbytes;
                n = n - nbytes;
            }
            if (n != 0)
            {
                std::memcpy(buffer_.data(), source, n);
            }
        }

        void ByteBuffer::get(std::size_t offset, void* dest, std::size_t n) const
        {
            CR_ASSERT(dest != nullptr && offset + n <= size_)(dest)(offset)(n)(size_);
            if (n != 0)
            {
                std::size_t readerIndex = (readerIndex_ + offset) % buffer_.size();
                std::size_t nbytes = std::min(buffer_.size() - readerIndex, n);
                std::memcpy(dest, buffer_.data() + readerIndex, nbytes);
                dest = static_cast<char*>(dest) + nbytes;
                n = n - nbytes;
            }
            if (n != 0)
            {
                std::memcpy(dest, buffer_.data(), n);
            }
        }

        void ByteBuffer::swap(ByteBuffer& other)
        {
            std::swap(buffer_, other.buffer_);
            std::swap(readerIndex_, other.readerIndex_);
            std::swap(writerIndex_, other.writerIndex_);
            std::swap(size_, other.size_);
        }

        void ByteBuffer::clear()
        {
            readerIndex_ = 0;
            writerIndex_ = 0;
            size_ = 0;
        }

        void ByteBuffer::ensure(std::size_t n)
        {
            if (buffer_.size() - size_ < n)
            {
                std::size_t capacity = std::max(static_cast<std::size_t>(buffer_.size() * 1.5), size_ + n);
                std::vector<char> buffer(capacity);
                boost::asio::buffer_copy(boost::asio::buffer(buffer), data());
                std::swap(buffer_, buffer);
                readerIndex_ = 0;
                writerIndex_ = size_ % buffer_.size();
            }
        }
    }
}
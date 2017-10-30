#include "byte_buffer.h"

#include <cstring>

#include <cr/core/assert.h>

namespace cr
{
    namespace network
    {
        ByteBuffer::ByteBuffer(std::size_t initialCapacity/* = 1024*/)
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

        std::size_t ByteBuffer::size() const
        {
            return size_;
        }

        ByteBuffer::ConstBuffers ByteBuffer::data() const
        {
            return data(0, size_);
        }

        ByteBuffer::MutableBuffers ByteBuffer::data()
        {
            return data(0, size_);
        }

        ByteBuffer::ConstBuffers ByteBuffer::data(std::size_t index, std::size_t n) const
        {
            CR_ASSERT(index + n <= size_)(index)(n)(size_)(readerIndex_)(writerIndex_);
            ConstBuffers buffers;
            if (n != 0)
            {
                std::size_t readerIndex = (readerIndex_ + index) % buffer_.size();
                if (writerIndex_ <= readerIndex)
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
            }
            return buffers;
        }

        ByteBuffer::MutableBuffers ByteBuffer::data(std::size_t index, std::size_t n)
        {
            CR_ASSERT(index + n <= size_)(index)(n)(size_)(readerIndex_)(writerIndex_);
            MutableBuffers buffers;
            if (n != 0)
            {
                std::size_t readerIndex = (readerIndex_ + index) % buffer_.size();
                if (writerIndex_ <= readerIndex)
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
            }
            return buffers;
        }

        void ByteBuffer::consume(std::size_t n)
        {
            CR_ASSERT(n <= size_)(n)(size_)(readerIndex_)(writerIndex_);
            if (n != 0)
            {
                readerIndex_ = (readerIndex_ + n) % buffer_.size();
                size_ = size_ - n;
            }
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
            if (n != 0)
            {
                writerIndex_ = (writerIndex_ + n) % buffer_.size();
                size_ = size_ + n;
            }
        }

        void ByteBuffer::uncommit(std::size_t n)
        {
            CR_ASSERT(n <= size_)(n)(size_)(buffer_.size())(readerIndex_)(writerIndex_);
            size_ -= n;
            if (writerIndex_ <= readerIndex_ && n != 0)
            {
                std::size_t nbytes = std::min(writerIndex_, n);
                writerIndex_ -= nbytes;
                n -= nbytes;
            }
            if (n != 0)
            {
                writerIndex_ = (writerIndex_ != 0 ? writerIndex_ : buffer_.size()) - n;
            }
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

        void ByteBuffer::shrink(std::size_t n/* = 1024*/)
        {
            if (n < buffer_.size())
            {
                std::size_t capacity = std::max(size_, n);
                std::vector<char> buffer(capacity);
                boost::asio::buffer_copy(boost::asio::buffer(buffer), data());
                std::swap(buffer_, buffer);
                readerIndex_ = 0;
                writerIndex_ = (size_ != buffer_.size()) ? size_ : 0;
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
                writerIndex_ = (size_ != buffer_.size()) ? size_ : 0;
            }
        }
    }
}
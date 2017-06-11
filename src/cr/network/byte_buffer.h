#ifndef CR_NETWORK_BYTE_BUFFER_H_
#define CR_NETWORK_BYTE_BUFFER_H_

#include <cstddef>
#include <vector>

#include <boost/asio/buffer.hpp>
#include <boost/container/static_vector.hpp>

namespace cr
{
    namespace network
    {
        /** 字节缓冲区 */
        class ByteBuffer
        {
        public:

            /** 可写缓冲区 */
            using MutableBuffers = boost::container::static_vector<boost::asio::mutable_buffer, 2>;

            /** 可读缓冲区 */
            using ConstBuffers = boost::container::static_vector<boost::asio::const_buffer, 2>;

            /**
             * 构造指定容量大小的缓冲区
             * @param initialCapacity 初始大小
             */
            explicit ByteBuffer(std::size_t initialCapacity = 1024);

            /**
             * 移动构造函数
             * @param other 待移动对象
             */
            ByteBuffer(ByteBuffer&& other);

            /**
             * 拷贝构造函数
             * @param other 待拷贝对象
             */
            ByteBuffer(const ByteBuffer& other) = default;

            /** 析构函数 */
            ~ByteBuffer();

            /**
             * 移动赋值函数
             * @param other 待移动对象
             */
            ByteBuffer& operator=(ByteBuffer&& other);

            /**
             * 拷贝赋值函数
             * @param other 待拷贝对象
             */
            ByteBuffer& operator=(const ByteBuffer& other) = default;

            /**
             * 缓冲区容量
             * @return 缓冲区容量
             */
            std::size_t getCapacity() const;

            /**
             * 可读字节数
             * @return 可读字节数
             */
            std::size_t getReadableBytes() const;

            /**
             * 可读数据
             * @param 可读数据缓冲区
             */
            ConstBuffers data() const;

            /**
             * 可读数据
             * @param 可读数据缓冲区
             */
            MutableBuffers data();

            /**
             * 可读数据
             * @param n 待读取字节数, n <= getReadableBytes()
             * @param 可读数据缓冲区
             */
            ConstBuffers data(std::size_t n) const;

            /**
             * 可读数据
             * @param n 待读取字节数, n <= getReadableBytes()
             * @param 可读数据缓冲区
             */
            MutableBuffers data(std::size_t n);

            /**
             * 消耗缓冲区数据
             * @param n 消耗字节数, n <= getReadableBytes()
             */
            void consume(std::size_t n);

            /**
             * 分配可写空间
             * @param n 分配字节数
             * @retrun 可写缓冲区序列
             */
            MutableBuffers prepare(std::size_t n);

            /**
             * 提交写入自己数
             * @param n 写入字节数
             */
            void commit(std::size_t n);

            /**
             * 随机覆写一段缓冲区
             * @param  offset 偏移量
             * @param b 源缓冲区
             * @param n 写入字节数
             */
            void set(std::size_t offset, const void* source, std::size_t n);

            /**
             * 随机读取一段缓冲区
             * @param  offset 偏移量
             * @param b 源缓冲区
             * @param n 写入字节数
             */
            void get(std::size_t offset, void* dest, std::size_t n) const;

            /**
             * 交换两个缓冲区
             * @param other 待交换缓冲区
             */
            void swap(ByteBuffer& other);

            /**
             * 清空缓冲区
             */
            void clear();

        private:

            //确保n个字节可写入
            void ensure(std::size_t n);

            std::vector<char> buffer_;
            std::size_t readerIndex_;
            std::size_t writerIndex_;
            std::size_t size_;
        };
    }
}

#endif
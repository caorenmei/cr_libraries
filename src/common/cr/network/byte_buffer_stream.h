#ifndef CR_COMMON_NETWORK_BYTE_BUFFER_STREAM_H_
#define CR_COMMON_NETWORK_BYTE_BUFFER_STREAM_H_

#include <cstddef>
#include <vector>

#include <boost/asio/buffer.hpp>
#include <google/protobuf/io/zero_copy_stream.h>

#include "byte_buffer.h"

namespace cr
{
    namespace network
    {
        /** Protobuf 输入流 */
        class ByteBufferInputStream : public google::protobuf::io::ZeroCopyInputStream
        {
        public:

            /**
             * 构造函数
             * @param b 缓冲区
             */
            explicit ByteBufferInputStream(const ByteBuffer::ConstBuffers& b);

            /**
             * 构造函数
             * @param b 缓冲区
             */
            explicit ByteBufferInputStream(const ByteBuffer::MutableBuffers& b);

            /** 析构函数 */
            ~ByteBufferInputStream();

            /**
             * 读取缓冲区
             * @param data 输出指针
             * @param size 输出字节数
             * @return 是否结束
             */
            virtual bool Next(const void** data, int* size) override;

            /**
             * 回退字节
             * @param count 回退字节数
             */
            virtual void BackUp(int count) override;

            /**
             * 跳过字节
             * @param count 跳过字节数
             * @return 是否结束
             */
            virtual bool Skip(int count) override;

            /**
             * 获取输入字节数
             * @return 输入字节数
             */
            virtual google::protobuf::int64 ByteCount() const override;

        private:

            // 缓冲区
            ByteBuffer::ConstBuffers buffers_;
            // 迭代器
            ByteBuffer::ConstBuffers::iterator iter_;
            // 读取索引
            std::size_t index_;
            // 输入字节数
            std::size_t count_;
        };

        /** Protobuf 输入流 */
        class ByteBufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream
        {
        public:

            /**
             * 构造函数
             * @param b 缓冲区
             */
            explicit ByteBufferOutputStream(ByteBuffer& b);

            /** 析构函数 */
            ~ByteBufferOutputStream();

            /**
             * 读取缓冲区
             * @param data 输出指针
             * @param size 输出字节数
             * @return 是否结束
             */
            virtual bool Next(void** data, int* size) override;

            /**
             * 回退字节
             * @param count 回退字节数
             */
            virtual void BackUp(int count) override;

            /**
             * 获取输入字节数
             * @return 输入字节数
             */
            virtual google::protobuf::int64 ByteCount() const override;

        private:

            // 缓冲区
            ByteBuffer& buffer_;
            // 输入字节数
            std::size_t originSize_;
        };
    }
}

#endif

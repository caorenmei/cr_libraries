#ifndef CR_COMMON_CONSISTENT_HASH_H_
#define CR_COMMON_CONSISTENT_HASH_H_

#include <cstdint>
#include <functional>
#include <type_traits>

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/crc.hpp>
#include <boost/optional.hpp>

namespace cr
{
    /** 一致性hash算法 */
    template <typename T, typename Hash = std::hash<T>, typename EqualTo = std::equal_to<T>>
    class ConsistentHash
    {
    public:

        /**
         * 构造函数
         * @param virtualNodeCount 虚拟节点数量
         */
        explicit ConsistentHash(std::uint32_t virtualNodeCount = 32)
            : virtualNodeCount_(virtualNodeCount),
            size_(0)
        {}

        /**
         * 添加 一个元素
         * @param element 元素
         * @param 是否添加成功
         */
        bool add(const T& element)
        {
            bool success = false;
            bool inserted = false;
            auto hashCode = hash_(element);
            for (std::size_t i = 0; i != virtualNodeCount_; ++i)
            {
                boost::crc_32_type crc32;
                crc32.process_bytes(&hashCode, sizeof(hashCode));
                crc32.process_bytes(&i, sizeof(i));
                auto checksum = crc32.checksum();
                std::tie(std::ignore, inserted) = elements_.left.insert(std::make_pair(checksum, element));
                if (!success && inserted)
                {
                    success = inserted;
                    size_ += 1;
                }
            }
            return success;
        }

        /**
         * 移除一个元素
         * @param element 待移除元素
         */
        void remove(const T& element)
        {
            auto iterRange = elements_.right.equal_range(element);
            if (iterRange.first != iterRange.second)
            {
                elements_.right.erase(iterRange.first, iterRange.second);
                size_ -= 1;
            }
        }

        /**
         * 是否包含一个元素
         * @param element 包含的元素
         * @return true包含，false其他
         */
        bool contains(const T& element) const
        {
            auto iter = elements_.right.find(element);
            return iter != elements_.right.end();
        }

        /**
         * 匹配字符串
         * @param key 匹配字符串
         * @return 匹配的元素 
         */
        boost::optional<const T&> match(const std::string& key) const
        {
            return match(key.data(), key.size());
        }

        /**
         * 匹配POD对象
         * @param key POD字符串
         * @return 匹配的元素 
         */
        template <typename U>
        std::enable_if_t<std::is_pod<U>::value, boost::optional<const T&>> 
            match(const U& element) const
        {
            return match(&element, sizeof(element));
        }

        /**
         * 匹配二进制流
         * @param buf 缓冲区起始地址
         * @param n 缓冲区长度
         * @return 匹配的元素 
         */
        boost::optional<const T&> match(const void* buf, std::size_t n) const
        {
            boost::crc_32_type crc32;
            crc32.process_bytes(buf, n);
            auto checksum = crc32.checksum();
            auto iter = elements_.left.lower_bound(checksum);
            if (iter == elements_.left.end())
            {
                iter = elements_.left.begin();
            }
            if (iter != elements_.left.end())
            {
                return { iter->second };
            }
            return {};
        }

        /**
         * 情况
         */
        void clear()
        {
            elements_.left.clear();
        }

        /** 
         * 判断是否为空
         * @param true为空，false其他
         */
        bool empty() const
        {
            return size_ == 0;
        }

        /**
         * 元素个数
         * @return 元素个数
         */
        std::size_t size() const
        {
            return  size_;
        }

    private:

        // 节点数据
        boost::bimap<
            boost::bimaps::set_of<std::uint32_t>,
            boost::bimaps::unordered_multiset_of<T, Hash, EqualTo>
        > elements_;
        // 元素hash函数
        Hash hash_;
        // 虚拟节点个数
        std::uint32_t virtualNodeCount_;
        // 元素个数
        std::size_t size_;
    };
}

#endif

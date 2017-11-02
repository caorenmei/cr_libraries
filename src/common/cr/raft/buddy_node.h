#ifndef CR_COMMON_RAFT_BUDDY_NODE_H_
#define CR_COMMON_RAFT_BUDDY_NODE_H_

#include <cstdint>

namespace cr
{
    namespace raft
    {
        /** 伙伴节点 */
        class BuddyNode
        {
        public:

            /**
             * 构造函数
             * @param nodeId 节点Id
             */
            explicit BuddyNode(std::uint64_t nodeId);

            /** 析构函数 */
            ~BuddyNode();

            /** 
             * 获取节点Id 
             * @return 节点Id
             */
            std::uint64_t getNodeId() const;

            /**
             * 设置下一个发送的索引
             * @param nextIndex 下一个发送的所以
             */
            void setNextIndex(std::uint64_t nextIndex);

            /**
             * 获取下一个发送的索引
             * @return 下一个发送的所以
             */
            std::uint64_t getNextIndex() const;

            /**
             * 设置等待回复的索引
             * @param waitIndex 等待回复的索引
             */
            void setWaitIndex(std::uint64_t waitIndex);

            /**
             * 获取等待回复的索引
             * @return 等待回复的索引
             */
            std::uint64_t getWaitIndex() const;

            /**
             * 设置匹配的索引
             * @param matchIndex 匹配的索引
             */
            void setMatchIndex(std::uint64_t matchIndex);

            /**
             * 获取匹配的索引
             * @return 匹配的索引
             */
            std::uint64_t getMatchIndex() const;

        private:

            // 节点Id
            std::uint64_t nodeId_;
            // 发送索引
            std::uint64_t nextIndex_;
            // 等待回复的索引
            std::uint64_t waitIndex_;
            // 匹配索引
            std::uint64_t matchIndex_;
        };
    }
}

#endif

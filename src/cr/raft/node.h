#ifndef CR_RAFT_NODE_H_
#define CR_RAFT_NODE_H_

#include <cstdint>

namespace cr
{
    namespace raft
    {
        /**
         * 代表一个Raft节点
         */
        class Node
        {
        public:

            /**
             * 构造函数
             * @param id 实例ID
             */
            explicit Node(std::uint32_t id);

            /** 析构函数 */
            ~Node();

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;

            /**
             * 获取节点ID
             * @return 节点ID
             */
            std::uint32_t getId() const;

        private:

            // 节点ID
            std::uint32_t id_;
        };
    }
}

#endif

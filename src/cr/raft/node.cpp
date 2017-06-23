#include <cr/raft/node.h>

namespace cr
{
    namespace raft
    {
        Node::Node(std::uint32_t id)
            : id_(id)
        {}

        Node::~Node()
        {}

        std::uint32_t Node::getId() const
        {
            return id_;
        }
    }
}

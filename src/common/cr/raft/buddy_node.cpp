#include "buddy_node.h"

namespace cr
{
    namespace raft
    {

        BuddyNode::BuddyNode(std::uint64_t nodeId)
            : nodeId_(nodeId),
            nextIndex_(1),
            waitIndex_(0),
            matchIndex_(0)
        {}

        BuddyNode::~BuddyNode()
        {}

        std::uint64_t BuddyNode::getNodeId() const
        {
            return nodeId_;
        }

        void BuddyNode::setNextIndex(std::uint64_t nextIndex)
        {
            nextIndex_ = nextIndex;
        }

        std::uint64_t BuddyNode::getNextIndex() const
        {
            return nextIndex_;
        }

        void BuddyNode::setWaitIndex(std::uint64_t waitIndex)
        {
            waitIndex_ = waitIndex;
        }

        std::uint64_t BuddyNode::getWaitIndex() const
        {
            return waitIndex_;
        }

        void BuddyNode::setMatchIndex(std::uint64_t matchIndex)
        {
            matchIndex_ = matchIndex;
        }

        std::uint64_t BuddyNode::getMatchIndex() const
        {
            return matchIndex_;
        }
    }
}
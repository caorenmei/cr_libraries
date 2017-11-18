#include "data_node.h"

#include <tuple>

namespace cr
{
    namespace cluster
    {
        DataNode::DataNode(std::string parent, std::string name, std::string data, const DataNodeStat& stat)
            : parent_(std::move(parent)),
            name_(std::move(name)),
            data_(std::move(data)),
            stat_(stat)
        {}

        DataNode::~DataNode()
        {}

        const std::string& DataNode::getParent() const
        {
            return parent_;
        }

        const std::string& DataNode::getName() const
        {
            return name_;
        }

        void DataNode::setData(std::string data)
        {
            data_ = std::move(data);
        }

        const std::string& DataNode::getData() const
        {
            return data_;
        }

        void DataNode::setStat(const DataNodeStat& stat)
        {
            stat_ = stat;
        }

        DataNodeStat& DataNode::getStat()
        {
            return stat_;
        }

        const DataNodeStat& DataNode::getStat() const
        {
            return stat_;
        }

        bool DataNode::addChild(const std::string& child)
        {
            bool success;
            std::tie(std::ignore, success) = children_.insert(child);
            return success;
        }

        bool DataNode::removeChild(const std::string& child)
        {
            auto iter = children_.find(child);
            if (iter != children_.end())
            {
                children_.erase(iter);
                return true;
            }
            return false;
        }

        void DataNode::setChildren(std::set<std::string> children)
        {
            children_ = std::move(children);
        }

        std::set<std::string>& DataNode::getChildren()
        {
            return children_;
        }

        const std::set<std::string>& DataNode::getChildren() const
        {
            return children_;
        }
    }
}
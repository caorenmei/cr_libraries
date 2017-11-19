#include "data_tree.h"

#include <cassert>

#include "exceptions.h"

namespace cr
{
    namespace zk
    {
        DataTree::DataTree()
            : versionIndex_(0)
        {
            root_ = std::make_shared<DataNode>("", "", "", DataNodeStat());
            nodes_.insert(std::make_pair("", root_));
        }

        DataTree::~DataTree()
        {}

        std::shared_ptr<DataNode> DataTree::getNode(const std::string& path) const
        {
            auto iter = nodes_.find(path);
            if (iter == nodes_.end())
            {
                return iter->second;
            }
            return nullptr;
        }


        std::string DataTree::createNode(const std::string& path, std::string data, const boost::optional<boost::uuids::uuid>& owner)
        {
            auto lastSlash = path.find_last_of('/');
            auto parentName = path.substr(0, lastSlash);
            auto childName = path.substr(lastSlash + 1);
            auto parentIter = nodes_.find(parentName);
            if (parentIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto parent = parentIter->second;
            auto& children = parent->getChildren();
            if (children.count(childName) != 0)
            {
                throw NodeExistsExeption();
            }
            versionIndex_ = versionIndex_ + 1;
            parent->addChild(childName);
            auto& parentStat = parent->getStat();
            parentStat.setCversion(versionIndex_);
            DataNodeStat stat;
            stat.setVersion(versionIndex_);
            stat.setDversion(versionIndex_);
            stat.setCversion(versionIndex_);
            stat.setOwner(owner);
            auto child = std::make_shared<DataNode>(parentName, childName, std::move(data), stat);
            nodes_.insert(std::make_pair(path, child));
            if (owner.is_initialized())
            {
                ephemerals_[*owner].insert(path);
            }
            dataWatches_.triggerWatch(path, EventType::NODE_CREATED);
            childWatches_.triggerWatch(parentName.empty() ? "/" : parentName, EventType::NODE_CHILDREN_CHANGED);
            return path;
        }

        void DataTree::deleteNode(const std::string& path, std::uint64_t version)
        {
            auto lastSlash = path.find_last_of('/');
            auto parentName = path.substr(0, lastSlash);
            auto childName = path.substr(lastSlash + 1);
            auto parentIter = nodes_.find(parentName);
            if (parentIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto parent = parentIter->second;
            auto& parentStat = parent->getStat();
            auto& children = parent->getChildren();
            if (children.count(childName) != 0)
            {
                throw NodeExistsExeption();
            }
            auto childIter = nodes_.find(path);
            assert(childIter != nodes_.end());
            auto child = childIter->second;
            auto& childStat = child->getStat();
            if (version != -1 && version != childStat.getVersion())
            {
                throw VersionException();
            }
            versionIndex_ = versionIndex_ + 1;
            children.erase(childName);
            parentStat.setCversion(versionIndex_);
            nodes_.erase(childIter);
            auto processed = dataWatches_.triggerWatch(path, EventType::NODE_DELETE);
            childWatches_.triggerWatch(path, EventType::NODE_DELETE, processed);
            childWatches_.triggerWatch(parentName.empty() ? "/" : parentName, EventType::NODE_CHILDREN_CHANGED);
        }

        DataNodeStat DataTree::setData(const std::string& path, std::string data, std::uint64_t version)
        {
            auto nodeIter = nodes_.find(path);
            if (nodeIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto node = nodeIter->second;
            auto& nodeStat = node->getStat();
            if (version != -1 && version != nodeStat.getDversion())
            {
                throw VersionException();
            }
            versionIndex_ = versionIndex_ + 1;
            node->setData(std::move(data));
            nodeStat.setDversion(versionIndex_);
            dataWatches_.triggerWatch(path, EventType::NODE_DATA_CHANGED);
            return nodeStat;
        }

        std::string DataTree::getData(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher)
        {
            auto nodeIter = nodes_.find(path);
            if (nodeIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto node = nodeIter->second;
            auto& nodeStat = node->getStat();
            if (version != -1 && version != nodeStat.getDversion())
            {
                throw VersionException();
            }
            if (watcher != nullptr)
            {
                dataWatches_.addWatcher(path, watcher);
            }
            return node->getData();
        }

        DataNodeStat DataTree::statNode(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher)
        {
            auto nodeIter = nodes_.find(path);
            if (nodeIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto node = nodeIter->second;
            auto& nodeStat = node->getStat();
            if (version != -1 && version != nodeStat.getDversion())
            {
                throw VersionException();
            }
            if (watcher != nullptr)
            {
                dataWatches_.addWatcher(path, watcher);
            }
            return nodeStat;
        }

        std::vector<std::string> DataTree::getChildren(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher)
        {
            auto nodeIter = nodes_.find(path);
            if (nodeIter == nodes_.end())
            {
                throw NoNodeExeption();
            }
            auto node = nodeIter->second;
            auto& nodeStat = node->getStat();
            if (version != -1 && version != nodeStat.getVersion())
            {
                throw VersionException();
            }
            if (watcher != nullptr)
            {
                childWatches_.addWatcher(path, watcher);
            }
            auto& children = node->getChildren();
            return { children.begin(), children.end() };
        }

        std::set<std::string> DataTree::getEphemerals(const boost::uuids::uuid& owner) const
        {
            auto iter = ephemerals_.find(owner);
            if (iter != ephemerals_.end())
            {
                return iter->second;
            }
            return {};
        }

        void DataTree::removeWatcher(const std::shared_ptr<Watcher>& watcher)
        {
            dataWatches_.removeWatcher(watcher);
            childWatches_.removeWatcher(watcher);
        }
    }
}
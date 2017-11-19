#ifndef CR_COMMON_ZK_NODE_TREE_H_
#define CR_COMMON_ZK_NODE_TREE_H_

#include <vector>

#include "create_model.h"
#include "data_node.h"
#include "watch_manager.h"

namespace cr
{
    namespace zk  
    {
        /** 节点树 */
        class DataTree
        {
        public:

            /** 构造函数 */
            DataTree();

            /** 析构函数 */
            ~DataTree();

            /**
             * 获取节点
             * @param path 路径
             * @return 节点
             */
            std::shared_ptr<DataNode> getNode(const std::string& path) const;


            /**
             * 创建节点
             * @param path 路劲
             * @param data 数据
             * @param owner 所有者
             * @return 路径
             * @throw NodeExistsExeption 节点已存在
             */
            std::string createNode(const std::string& path, std::string data, const boost::optional<boost::uuids::uuid>& owner);

            /**
             * 删除节点
             * @param path 路径
             * @param version 版本
             * @throw NoNodeExeption 节点不存在
             * @throw VersionException 版本不匹配
             */
            void deleteNode(const std::string& path, std::uint64_t version);

            /**
             * 设置数据
             * @param path 路径
             * @param data 数据
             * @param version 版本号
             * @return 状态
             * @throw NoNodeExeption 节点不存在
             * @throw VersionException 版本不匹配
             */
            DataNodeStat setData(const std::string& path, std::string data, std::uint64_t version);

            /**
             * 获取数据
             * @param path 路径
             * @param version 版本号
             * @param watcher 监视器
             * @return 数据
             * @throw NoNodeExeption 节点不存在
             * @throw VersionException 版本不匹配
             */
            std::string getData(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher);

            /**
             * 获取节点状态
             * @param path 路径
             * @param version 版本号
             * @param watcher 监视器
             * @return 节点状态
             * @throw NoNodeExeption 节点不存在
             * @throw VersionException 版本不匹配
             */
            DataNodeStat statNode(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher);

            /**
             * 获取子节点
             * @param path 路径
             * @param version 版本号
             * @param watcher 监视器
             * @return 子节点列表
             * @throw NoNodeExeption 节点不存在
             * @throw VersionException 版本不匹配
             */
            std::vector<std::string> getChildren(const std::string& path, std::uint64_t version, std::shared_ptr<Watcher> watcher);

            /**
             * 获取临时节点
             * @param owner 所有者
             * @return 临时节点列表
             */
            std::set<std::string> getEphemerals(const boost::uuids::uuid& owner) const;
            
            /**
             * 移除监视器
             * @param watcher 监视器
             */
            void removeWatcher(const std::shared_ptr<Watcher>& watcher);

        private:

            // 节点
            std::map<std::string, std::shared_ptr<DataNode>> nodes_;
            // 数据监视器
            WatchManager dataWatches_;
            // 子节点监视器
            WatchManager childWatches_;
            // 临时节点
            std::map<boost::uuids::uuid, std::set<std::string>> ephemerals_;
            // 版本
            std::uint64_t versionIndex_;
            // 根节点 
            std::shared_ptr<DataNode> root_;
        };
    }
}

#endif // !CR_COMMON_CLUSTER_NODE_TREE_H_


#ifndef CR_COMMON_ZK_DATA_NODE_H_
#define CR_COMMON_ZK_DATA_NODE_H_

#include <cstdint>
#include <set>
#include <string>

#include "data_node_stat.h"

namespace cr
{
    namespace zk
    {
        /** 数据节点 */
        class DataNode
        {
        public:

            /**
             * 构造函数
             * @param parent 父节点路径
             * @param name 自身节点名
             * @param data 节点数据
             * @param stat 节点属性
             */
            DataNode(std::string parent, std::string name, std::string data, const DataNodeStat& stat);

            /** 析构函数 */
            ~DataNode();

            /**
             * 获取父节点
             * @return 父节点路径
             */
            const std::string& getParent() const;

            /**
             * 获取节点名
             * @return 节点名
             */
            const std::string& getName() const;

            /**
             * 设置数据
             * @param data 数据
             */
            void setData(std::string data);

            /**
             * 获取数据
             * @return 数据
             */
            const std::string& getData() const;

            /**
             * 设置节点属性
             * @param stat 节点属性
             */
            void setStat(const DataNodeStat& stat);

            /**
             * 获取节点属性
             * @return 节点属性
             */
            DataNodeStat& getStat();
            const DataNodeStat& getStat() const;

            /**
             * 添加子节点
             * @param child 子节点
             * @return true成功，false其他
             */
            bool addChild(const std::string& child);

            /**
             * 移除子节点
             * @param child 子节点
             * @return true成功，false其他
             */
            bool removeChild(const std::string& child);

            /**
             * 设置子节点
             * @param children 子节点
             */
            void setChildren(std::set<std::string> children);

            /**
             * 获取子节点
             * @retrn 子节点
             */
            std::set<std::string>& getChildren();
            const std::set<std::string>& getChildren() const;

        private:

            // 父节点
            std::string parent_;
            // 自身节点
            std::string name_;
            // 数据
            std::string data_;
            // 属性
            DataNodeStat stat_;
            // 子节点
            std::set<std::string> children_;
        };
    }
}

#endif

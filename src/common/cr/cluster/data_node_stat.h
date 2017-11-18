#ifndef CR_COMMON_CLUSTER_DATA_NODE_STAT_H_
#define CR_COMMON_CLUSTER_DATA_NODE_STAT_H_

#include <cstdint>
#include <string>

namespace cr
{
    namespace cluster
    {
        /** 节点源信息 */
        class DataNodeStat
        {
        public:

            /** 构造函数 */
            DataNodeStat();

            /** 析构函数 */
            ~DataNodeStat();

            /**
             * 设置节点版本号
             * @param version 节点版本号
             */
            void setVersion(std::uint64_t version);

            /**
             * 获取节点版本号
             * @return 节点版本号
             */
            std::uint64_t getVersion() const;

            /**
             * 设置子节点版本号
             * @param version 子节点版本号
             */
            void setCversion(std::uint64_t cversion);

            /**
             * 获取子节点版本号
             * @return 子节点版本号
             */
            std::uint64_t getCversion() const;

            /**
             * 设置节点数据版本号
             * @param dversion 节点数据版本号
             */
            void setDversion(std::uint64_t dversion);

            /**
             * 获取节点数据版本号
             * @return 节点数据版本号
             */
            std::uint64_t getDversion() const;

            /**
             * 设置节点所有者
             * @param owner 节点所有者
             */
            void setOwner(std::string owner);

            /**
             * 获取节点所有者
             * @return 节点所有者
             */
            const std::string& getOwner() const;

        private:

            // 版本号, 节点版本
            std::uint64_t version_;
            // 版本号, 子节点
            std::uint64_t cversion_;
            // 版本号, 数据
            std::uint64_t dversion_;
            // 拥有者
            std::string owner_;
        };
    }
}

#endif

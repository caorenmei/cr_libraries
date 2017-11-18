#ifndef CR_COMMON_CLUSTER_NODE_VERSION_
#define CR_COMMON_CLUSTER_NODE_VERSION_

#include <cstdint>
#include <string>

#include <boost/uuid/uuid.hpp>

namespace cr
{
    namespace cluster
    {
        /** 节点版本号 */
        struct NodeVersion
        {
            /** 节点版本 */
            std::uint64_t version;
            /** 数据版本 */
            std::uint64_t dversion;
            /** 子节点版本 */
            std::uint64_t cversion;
        };

        /** 节点数结构 */
        struct ValueNode
        {
            /** 节点名 */
            std::string name;
            /** 父节点 */
        };
    }
}

#endif

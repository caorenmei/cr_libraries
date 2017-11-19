#ifndef CR_COMMON_CLUSTER_SERVICE_OPTIONS_H_
#define CR_COMMON_CLUSTER_SERVICE_OPTIONS_H_

#include <cr/raft/service_options.h>

namespace cr
{
    namespace app
    {
        /** 服务参数 */
        struct ServiceOptions : cr::raft::ServiceOptions
        {
            /** tick 时长*/
            std::uint64_t tickTime;
            /** 自动删除 tick */
            std::size_t deleteLimit;
        };
    }
}

#endif

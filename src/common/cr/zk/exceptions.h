#ifndef CR_COMMON_ZK_EXCEPTIONS_H_
#define CR_COMMON_ZK_EXCEPTIONS_H_

#include <cr/core/exception.h>

namespace cr
{
    namespace zk
    {
        /** 异常基类 */
        class ZkException : public cr::Exception
        {
        public:
            using cr::Exception::Exception;
        };
        /** 节点已存在异常 */
        class NodeExistsExeption : public ZkException
        {
        public:
            using ZkException::ZkException;
        };

        /** 节点不存在异常 */
        class NoNodeExeption : public ZkException
        {
        public:
            using ZkException::ZkException;
        };

        /** 版本不匹配异常 */
        class VersionException : public ZkException
        {
        public:
            using ZkException::ZkException;
        };
    }
}

#endif

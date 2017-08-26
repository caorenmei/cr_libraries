#ifndef CR_APP_LOCAL_CLUSTER_IMPL_H_
#define CR_APP_LOCAL_CLUSTER_IMPL_H_

#include <map>
#include <mutex>
#include <set>

#include <cr/concurrent/pipe.h>

#include "cluster.h"

namespace cr
{
    namespace app
    {

        class ClusterProxyImpl;

        /** 本地集群接口 */
        class LocalClusterImpl : public std::enable_shared_from_this<LocalClusterImpl>, public Cluster
        {
        public:

            /**
             * 构造函数
             * @param ioService 运行线程
             */
            explicit LocalClusterImpl(boost::asio::io_service& ioService);

            /** 析构函数 */
            virtual ~LocalClusterImpl() override;

            LocalClusterImpl(const LocalClusterImpl&) = delete;
            LocalClusterImpl& operator=(const LocalClusterImpl&) = delete;

            /**
             * 创建一个连接
             * @param ioService io service
             * @param name 服务名字
             * @param data 数据
             * @return 集群代理接口
             */
            virtual std::shared_ptr<ClusterProxy> connect(boost::asio::io_service& ioService, std::string name, std::string data) override;

        private:

            // 服务连接
            void onProxyConnect(std::uint32_t fromId, std::shared_ptr<ClusterProxyImpl> proxy, std::string name, std::string data);

            // proxy断开
            void onProxyDisconnect(std::uint32_t fromId);

            // 接受服务消息处理器
            void onProxyMessage(std::uint32_t fromId, std::uint32_t toId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

            // 接受服务观察
            void onProxyWatch(std::uint32_t fromId, std::string name);

            // 停止服务观察
            void onProxyUnWatch(std::uint32_t fromId, std::string name);

            // 服务数据改变
            void onProxyDataUpdate(std::uint32_t formId, std::string data);

            boost::asio::io_service& ioService_;
            // 下一个服务Id
            std::uint32_t nextId_;
            // 名字 <-> 服务
            std::map<std::string, std::set<std::uint32_t>> nameIds_;
            std::map<std::uint32_t, std::string> idNames_;
            // 服务 <-> 观察
            std::map<std::string, std::set<std::uint32_t>> watchNameIds_;
            std::map<std::uint32_t, std::set<std::string>> watchIdNames_;
            // 服务私有数据
            std::map<std::uint32_t, std::string> datas_;
            // 服务代理
            std::map<std::uint32_t, std::shared_ptr<ClusterProxyImpl>> proxies_;
        };
    }
}

#endif

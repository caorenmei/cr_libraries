#ifndef CR_APP_CLUSTER_PROXY_IMPL_H_
#define CR_APP_CLUSTER_PROXY_IMPL_H_

#include <map>
#include <mutex>
#include <set>

#include <cr/concurrent/pipe.h>

#include "cluster.h"

namespace cr
{
    namespace app
    {
        /** 本地集群代理 */
        class ClusterProxyImpl : public std::enable_shared_from_this<ClusterProxyImpl>, public ClusterProxy
        {
        public:

            /**
             * 构造函数
             * @param clusterIoService 主线程的ioService,即LocalCluster的io service
             * @param workIoService 工作线程的io service
             */
            ClusterProxyImpl(boost::asio::io_service& clusterIoService, boost::asio::io_service& workIoService, std::string name);

            /** 析构函数 */
            virtual ~ClusterProxyImpl() override;

            ClusterProxyImpl(const ClusterProxyImpl&) = delete;
            ClusterProxyImpl& operator=(const ClusterProxyImpl&) = delete;

            /**
             * 获取服务名字
             * @return 名字
             */
            virtual const std::string& getName() const override;

            /**
             * 获取服务Id
             * @return 服务Id
             */
            virtual std::uint32_t getId() const override;

            /**
             * 断开连接
             */
            virtual void disconnect() override;

             /**
             * 设置连接成功回调
             * @param handler 回调
             */
            virtual void onConnect(std::function<void(std::uint32_t)> handler) override;

            /**
             * 监视服务改变
             * @param name 服务名字
             * @param handler 事件处理器
             */
            virtual void watch(std::string name, std::function<void(Event, std::string, std::uint32_t, std::string)> handler) override;

            /**
             * 停止监视服务改变
             * @param name 服务名字
             */
            virtual void unwatch(std::string name) override;

            /**
             * 更新数据
             * @param data 数据
             */
            virtual void update(std::string data) override;

            /**
             * 设置消息处理器
             * @oaram handler 消息处理器
             */
            virtual void onMessageReceived(std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> handler) override;

            /**
             * 发送消息
             * @param id 目的服务Id
             * @param session 会话
             * @param message 消息
             */
            virtual void sendMessage(std::uint32_t id, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message) override;

            /**
             * 服务建立成功
             * @param id 服务Id
             */
            void onClusterConnect(std::uint32_t id);

            /**
             * 收到其它服务消息
             * @param id 源服务Id
             * @param session 会话
             * @param message 消息
             */
            void onClusterMessageReceive(std::uint32_t id, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

            /**
             * 服务改变
             * @param event 事件
             * @param name 服务名字
             * @param fromId 事件发生服务Id
             * @param data 数据
             */
            void onClusterWatchEvent(Event event, std::string name, std::uint32_t fromId, std::string data);
           
            /**
             * 服务断开回调
             * @param handler 回调函数
             */
            void setClusterDisconnectHander(std::function<void()> handler);

            /**
             * 服务监听通知回调
             * @param handler 回调函数
             */
            void setClusterWatchHander(std::function<void(std::string)> handler);

            /**
             * 服务停止监听通知回调
             * @param handler 回调函数
             */
            void setClusterUnWatchHander(std::function<void(std::string)> handler);

            /**
             * 服务数据更新通知回调
             * @param handler 回调函数
             */
            void setClusterDataUpdateHander(std::function<void(std::string)> handler);

            /**
             * 服务数据更新通知回调
             * @param handler 回调函数
             */
            void setClusterMessageHander(std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> handler);

        private:

            // 连接成功
            void onProxyConnect(std::uint32_t id);

            // 设置断开处理器
            void onSetClusterDisconnectHander(std::function<void()> handler);

            // 监听服务改变
            void onProxyWatchEvent(Event event, std::string name, std::uint32_t fromId, std::string data);

            // 判处来自集群的消息
            void popClusterMessage();

            // 集群消息回调
            void onPopClusterMessage();

            // 弹出来自代理的消息
            void popProxyMessage();

            // 代理消息回调
            void onPopProxyMessage();

            // 主线程 
            boost::asio::io_service& clusterIoService_;
            // 工作线程
            boost::asio::io_service& proxyIoService_;
            // 状态 
            enum State { NORMAL, CONNECTED, DISCONNECTED };
            State state_;
            // 名字
            std::string name_;
            // 服务Id
            std::uint32_t id_;
            // 主消息队列
            using QueueMessage = std::tuple<std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>>;
            cr::concurrent::Pipe<QueueMessage, std::mutex> clusterMessageQueue_;
            std::vector<QueueMessage> popClusterMessages_;
            // 工作消息队列
            cr::concurrent::Pipe<QueueMessage, std::mutex> proxyMessageQueue_;
            std::vector<QueueMessage> popProxyMessages_;
            // 通知集群服务断开连接
            std::function<void()> clusterDisconnectHandler_;
            // 通知集群服务监听服务
            std::function<void(std::string)> clusterWatchHander_;
            // 通知集群停止监听服务
            std::function<void(std::string)> clusterUnWatchHander_;
            // 通知集群数据更新
            std::function<void(std::string)> clusterDataHandler_;
            // 集群消息监听
            std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> clusterMessageHandler_;
            // 连接建立回调
            std::function<void(std::uint32_t)> proxyConnectHandler_;
            // 名字监听回调
            std::map<std::string, std::function<void(Event, std::string, std::uint32_t, std::string)>> proxyWatchHandlers_;
            // 消息回调
            std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> proxyMessageHandler_;
        };
    }
}

#endif

#ifndef CR_APP_CLUSTER_H_
#define CR_APP_CLUSTER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio/io_service.hpp>

#include "message.h"

namespace cr
{
    namespace app
    {

        /** 集群代理接口 */
        class ClusterProxy;

        /** 集群接口 */
        class Cluster
        {
        public:

            /** 析构函数 */
            virtual ~Cluster() {}

            /**
             * 创建一个连接
             * @param ioService io service
             * @param name 服务名字
             * @param data 数据
             * @return 集群代理接口
             */
            virtual std::shared_ptr<ClusterProxy> connect(boost::asio::io_service& ioService, std::string name, std::string data) = 0;

            /**
             * 生成分布式下的唯一Id
             * @return 唯一Id
             */
            virtual std::uint64_t genUuid() = 0;

            /**
             * 集群是否有效
             * @return true 有效，false其他
             */
            virtual bool isValid() const = 0;
        };

        /** 集群代理接口 */
        class ClusterProxy
        {
        public:

            /** 集群事件 */
            enum Event
            {
                /** 新的节点 */
                ADD = 0,
                /** 节点更新 */
                UPDATE = 1,
                /** 节点移除 */
                REMOVE = 2,
            };

            /** 析构函数 */
            virtual ~ClusterProxy() {}

            /**
             * 获取服务名字
             * @return 名字
             */
            virtual const std::string& getName() const = 0;

            /**
             * 获取服务Id
             * @return 服务Id
             */
            virtual std::uint32_t getId() const = 0;

            /**
             * 断开连接
             */
            virtual void disconnect() = 0;

            /**
             * 设置连接成功回调
             * @param handler 回调
             */
            virtual void onConnect(std::function<void(std::uint32_t)> handler) = 0;

            /**
             * 监视服务改变
             * @param name 服务名字
             * @param handler 事件处理器
             */
            virtual void watch(std::string name, std::function<void(Event, std::string, std::uint32_t, std::string)> handler) = 0;

            /**
             * 停止监视服务改变
             * @param name 服务名字
             */
            virtual void unwatch(std::string name) = 0;

            /**
             * 更新数据
             * @param data 数据
             */
            virtual void update(std::string data) = 0;

            /**
             * 设置消息处理器
             * @oaram handler 消息处理器
             */
            virtual void onMessageReceived(std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> handler) = 0;

            /**
             * 发送消息
             * @param id 目的服务Id
             * @param session 会话
             * @param message 消息
             */
            virtual void sendMessage(std::uint32_t id, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message) = 0;

        };
    }
}

#endif

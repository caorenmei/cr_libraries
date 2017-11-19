#ifndef CR_COMMON_CLUSTER_NAME_SERVICE_H_
#define CR_COMMON_CLUSTER_NAME_SERVICE_H_

#include <map>
#include <set>

#include <boost/uuid/uuid.hpp>

#include <cr/raft/raft_service.h>

#include "name_service.h"
#include "name_service.pb.h"
#include "service_options.h"

namespace cr
{
    namespace app
    {

        /** 名字服务 */
        class NameService : public cr::raft::RaftService
        {
        public:

            /** 服务参数 */
            using Options = ServiceOptions;

            /**
             * 构造函数
             * @param context app
             * @param ioService 工作线程
             * @param id 服务Id
             * @param name 服务名字
             * @param options 服务参数
             */
            NameService(cr::app::Application& context, boost::asio::io_service& ioService,
                std::uint32_t id, std::string name, const Options& options);

            /** 析构函数 */
            ~NameService();

        protected:

            /** 开始服务 */
            virtual void onStart() override;

            /** 停止服务 */
            virtual void onStop() override;

            /**
            * 运行raft指令
            * @param index 日志index
            * @param value 日志值
            */
            virtual void onCommand(std::uint64_t index, const std::string& value) override;

        private:

			// 节点数据结构
			struct ValueNode;
			// 连接
			struct Connection;

            // 操作指令
            void onOpCommand(const pb::OpCommand& command);

            // 添加指令
            std::uint32_t onAddCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId, 
				const pb::OpCommand& command, std::shared_ptr<ValueNode>& node);

            // 更新指令
			std::uint32_t onUpdateCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId, 
				const pb::OpCommand& command, std::shared_ptr<ValueNode>& node);

            // 删除指令
			std::uint32_t onRemoveCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
				const pb::OpCommand& command, std::shared_ptr<ValueNode>& node);

			// 添加自动删除节点
			void addAutoDeleteNode(const std::shared_ptr<ValueNode>& node);

			// 移除自动删除节点
			void removeAutoDeleteNode(const std::shared_ptr<ValueNode>& node);

			// 自动删除临时节点
			void onAutoDeleteTick();

            // io service
            boost::asio::io_service& ioService_;
            // 日志
            cr::log::Logger& logger_;
            // 服务参数
            Options options_;
            // tick定时器
            boost::asio::steady_timer tickTimer_;
            // 下一个版本号
            std::uint64_t versionIndex_;
			// 节点数据结构
			struct ValueNode;
            // 节点树
            std::shared_ptr<ValueNode> root_;
            // 临时节点
            std::map<boost::uuids::uuid, std::pair<std::set<std::shared_ptr<ValueNode>>, std::size_t>> ephemerals_;
            // 接收器
            boost::asio::ip::tcp::acceptor acceptor_;
            boost::asio::ip::tcp::socket socket_;
            // 序号
            std::uint64_t nextId_;
            // 客户端连接
            std::map<boost::uuids::uuid, std::shared_ptr<Connection>> connections_;
        };
    }
}

#endif
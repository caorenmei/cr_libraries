#ifndef CR_COMMON_APP_NAME_SERVICE_H_
#define CR_COMMON_APP_NAME_SERVICE_H_

#include <map>
#include <set>

#include <boost/uuid/uuid.hpp>

#include "name_command.pb.h"
#include "name_op.pb.h"
#include "raft_service.h" 

namespace cr
{
    namespace app
    {
        /** 名字服务 */
        class NameService : public RaftService
        {
        public:

            /** 服务参数 */
            struct Options : RaftService::Options
            {
                /** tick 时长*/
                std::uint64_t tickTime;
                /** 自动删除 tick */
                std::size_t deleteLimit;
            };

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
            void onAddCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId, 
                const std::vector<std::string>& path, const pb::OpCommand& command);

            // 更新指令
            void onUpdateCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
                const std::vector<std::string>& path, const pb::OpCommand& command);

            // 删除指令
            void onRemoveCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
                const std::vector<std::string>& path, const pb::OpCommand& command);

            // 获取节点
            std::shared_ptr<ValueNode> getValueNode(const std::vector<std::string>& path) const;

            // 获取父节点
            std::shared_ptr<ValueNode> getParentValueNode(const std::vector<std::string>& path) const;

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
            // 节点树
            std::shared_ptr<ValueNode> root_;;
            // 临时节点
            std::map<boost::uuids::uuid, std::set<std::weak_ptr<ValueNode>>> ephemerals_;
            // 临时节点删除检测
            std::map<boost::uuids::uuid, std::size_t> autoDeleteTicks_;
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
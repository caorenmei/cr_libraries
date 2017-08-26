#include "req_service.h"

#include <cr/app/application.h>

#include "calc_service.h"
#include "unittest.pb.h"

ReqService::ReqService(cr::app::Application& context, boost::asio::io_service& ioService,
    std::uint32_t id, const std::string& name, std::atomic<std::uint32_t>& result)
    : cr::app::Service(context, ioService, id, name),
    result(result)
{}


void ReqService::onStart()
{
    cr::app::Service::onStart();
    auto serverId = getApplicationContext().startService<CalcService>("Thread0", ".CalcService").get();
    auto request = std::make_shared<proto::UInt32>();
    request->set_value(10);
    sendMessage(serverId, 0, request);
}

void ReqService::onMessageReceived(std::uint32_t serverId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
{
    if (message->GetDescriptor() == proto::UInt32::descriptor())
    {
        auto request = std::static_pointer_cast<proto::UInt32>(message);
        this->result = request->value();
        getApplicationContext().stop();
    }
}

ClusterReqService::ClusterReqService(cr::app::Application& context, boost::asio::io_service& ioService,
    std::uint32_t id, const std::string& name, std::atomic<std::uint32_t>& result)
    : cr::app::Service(context, ioService, id, name),
    result(result)
{}

void ClusterReqService::onStart()
{
    cr::app::Service::onStart();
    // 集群代理
    auto cluster = getCluster();
    proxy_ = cluster->connect(getIoService(), "ReqService", "hello");
    proxy_->onConnect([this, self = shared_from_this()](std::uint32_t myId)
    {
        onProxyConnect(myId);
    });
    proxy_->onMessageReceived([this, self = shared_from_this()](std::uint32_t fromId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
    {
        onProxyMessageReceived(fromId, session, std::move(message));
    });
}

void ClusterReqService::onStop()
{
    cr::app::Service::onStart();
    proxy_->disconnect();
}

void ClusterReqService::onProxyConnect(std::uint32_t myId)
{
    proxy_->watch("CalcService", [this](cr::app::ClusterProxy::Event event, std::string name, std::uint32_t fromId, std::string data)
    {
        onProxyWatchEvent(event, name, fromId, std::move(data));
    });
}

void ClusterReqService::onProxyWatchEvent(cr::app::ClusterProxy::Event event, std::string name, std::uint32_t fromId, std::string data)
{
    if (event != cr::app::ClusterProxy::REMOVE && name == "CalcService")
    {
        auto request = std::make_shared<proto::UInt32>();
        request->set_value(10);
        proxy_->sendMessage(fromId, 0, request);
    }
}

void ClusterReqService::onProxyMessageReceived(std::uint32_t serverId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
{
    if (message->GetDescriptor() == proto::UInt32::descriptor())
    {
        auto request = std::static_pointer_cast<proto::UInt32>(message);
        this->result = request->value();
        getApplicationContext().stop();
    }
}
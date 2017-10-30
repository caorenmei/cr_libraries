#include "calc_service.h"

#include <numeric>

#include <cr/app/application.h>

#include "unittest.pb.h"

void CalcService::onStart()
{
    cr::app::Service::onStart();
}

void CalcService::onMessageReceived(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
{
    if (message->GetDescriptor() == proto::UInt32::descriptor())
    {
        auto request = std::static_pointer_cast<proto::UInt32>(message);
        auto response = std::make_shared<proto::UInt32>();
        response->set_value(request->value() * request->value());
        sendMessage(serviceId, session, std::move(response));
    }
}

void ClusterCalcService::onStart()
{
    cr::app::Service::onStart();
    // 集群代理
    auto cluster = getCluster();
    proxy_ = cluster->connect(getIoService(), "CalcService", "hello");
    proxy_->onConnect([this, self = shared_from_this()](std::uint32_t myId)
    {
        onProxyConnect(myId);
    });
    proxy_->onMessageReceived([this, self = shared_from_this()](std::uint32_t fromId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
    {
        onProxyMessageReceived(fromId, session, std::move(message));
    });
}

void ClusterCalcService::onStop()
{
    proxy_->disconnect();
}

void ClusterCalcService::onProxyConnect(std::uint32_t myId)
{

}

void ClusterCalcService::onProxyMessageReceived(std::uint32_t fromId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
{
    if (message->GetDescriptor() == proto::UInt32::descriptor())
    {
        auto request = std::static_pointer_cast<proto::UInt32>(message);
        auto response = std::make_shared<proto::UInt32>();
        response->set_value(request->value() * request->value());
        proxy_->sendMessage(fromId, session, std::move(response));
    }
}
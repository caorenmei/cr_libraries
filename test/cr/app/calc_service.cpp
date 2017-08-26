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
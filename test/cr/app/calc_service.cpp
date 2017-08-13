#include "calc_service.h"

#include <numeric>

#include <cr/app/application.h>

#include "unittest.pb.h"

void CalcService::onStart()
{
    cr::app::Service::onStart();
    getApplicationContext().getCluster()->registerService(".CalcService", getId());
}

void CalcService::onMessageReceived(std::uint32_t sourceId, std::uint32_t sourceServiceId, std::uint64_t session, std::shared_ptr<cr::app::Message> message)
{
    if (message->GetDescriptor() == AccumulateReq::descriptor())
    {
        auto request = std::static_pointer_cast<AccumulateReq>(message);
        auto result = std::accumulate(request->arguments().begin(), request->arguments().end(), 0);
        auto response = std::make_shared<AccumulateResp>();
        response->set_result(result);
        sendMessage(sourceId, sourceServiceId, session, response);
    }
}
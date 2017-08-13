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
    getNames(".CalcService");
    getApplicationContext().startService<CalcService>("Thread0", ".CalcService");
}

void ReqService::onMessageReceived(std::uint32_t sourceId, std::uint32_t sourceServiceId, std::uint64_t session, std::shared_ptr<cr::app::Message> message) 
{
    if (message->GetDescriptor() == AccumulateResp::descriptor())
    {
        auto response = std::static_pointer_cast<AccumulateResp>(message);
        this->result = response->result();
        getApplicationContext().stop();
    }
}

void ReqService::getNames(const std::string& name)
{
    std::weak_ptr<ReqService> self = std::static_pointer_cast<ReqService>(shared_from_this());
    auto hostIds = getApplicationContext().getCluster()->getNames(name, getIoService().wrap([this, self, name]
    {
        auto service = self.lock();
        if (this->watch && service)
        {
            service->getNames(name);
        }
    }));
    for (auto hostId : hostIds)
    {
        getServices(name, hostId);
    }
}

void ReqService::getServices(const std::string& name, std::uint32_t hostId)
{
    std::weak_ptr<ReqService> self = std::static_pointer_cast<ReqService>(shared_from_this());
    auto serviceIds = getApplicationContext().getCluster()->getServices(name, hostId, getIoService().wrap([this, self, name, hostId]
    {
        auto service = self.lock();
        if (this->watch && service)
        {
            service->getServices(name, hostId);
        }
    }));
    // 获取到了服务Id
    if (this->watch && serviceIds.second && !serviceIds.first.empty())
    {
        this->watch = false;
        auto request = std::make_shared<AccumulateReq>();
        for (std::uint32_t i = 0; i < 10; ++i)
        {
            request->add_arguments(i);
        }
        sendMessage(hostId, serviceIds.first[0], 0, request);
    }
}
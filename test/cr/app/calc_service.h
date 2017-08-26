#ifndef CR_APP_CALC_SERVICE_H_
#define CR_APP_CALC_SERVICE_H_

#include <cr/app/service.h>

class CalcService : public cr::app::Service
{
public:

    using cr::app::Service::Service;

protected:

    virtual void onStart() override;

    virtual void onMessageReceived(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message) override;
};

class ClusterCalcService : public cr::app::Service
{
public:

    using cr::app::Service::Service;

protected:

    virtual void onStart() override;

    virtual void onStop() override;

private:

    void onProxyConnect(std::uint32_t myId);

    void onProxyMessageReceived(std::uint32_t fromId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

    std::shared_ptr<cr::app::ClusterProxy> proxy_;
};

#endif // !CR_APP_CALC_SERVICE_H_


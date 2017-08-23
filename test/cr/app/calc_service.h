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

#endif // !CR_APP_CALC_SERVICE_H_


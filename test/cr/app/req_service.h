#ifndef CR_APP_REQ_SERVICE_H_
#define CR_APP_REQ_SERVICE_H_

#include <atomic>

#include <cr/app/service.h>

class ReqService : public cr::app::Service
{
public:

    ReqService(cr::app::Application& context, boost::asio::io_service& ioService,
        std::uint32_t id, const std::string& name, std::atomic<std::uint32_t>& result);

protected:

    virtual void onStart() override;

    virtual void onMessageReceived(std::uint32_t serverId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message) override;

private:

    std::atomic<std::uint32_t>& result;
};

#endif // !CR_APP_REQ_SERVICE_H_

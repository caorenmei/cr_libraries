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

class ClusterReqService : public cr::app::Service
{
public:

    ClusterReqService(cr::app::Application& context, boost::asio::io_service& ioService,
        std::uint32_t id, const std::string& name, std::atomic<std::uint32_t>& result);

protected:

    virtual void onStart() override;

    virtual void onStop() override;

private:
    void onProxyConnect(std::uint32_t myId);

    void onProxyWatchEvent(cr::app::ClusterProxy::Event event, std::string name, std::uint32_t fromId, std::string data);

    void onProxyMessageReceived(std::uint32_t fromId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

    std::shared_ptr<cr::app::ClusterProxy> proxy_;
    std::atomic<std::uint32_t>& result;
};

#endif // !CR_APP_REQ_SERVICE_H_

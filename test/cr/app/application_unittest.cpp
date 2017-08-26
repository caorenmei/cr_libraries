#include <boost/test/unit_test.hpp>

#include <atomic>
#include <numeric>

#include <cr/app/application.h>
#include <cr/app/service.h>

#include "calc_service.h"
#include "req_service.h"
#include "unittest.pb.h"

BOOST_AUTO_TEST_SUITE(Application)

BOOST_AUTO_TEST_CASE(CalcApplication)
{

    boost::asio::io_service ioService;
    auto app = std::make_shared<cr::app::Application>(ioService);
    app->start();

    std::atomic<std::uint32_t> result(0);
    app->startService<ReqService>("Thread1", ".ReqService", std::ref(result));
    
    ioService.run();

    BOOST_CHECK_EQUAL(result, 100);
}

BOOST_AUTO_TEST_CASE(CalcCluster)
{

    boost::asio::io_service ioService;
    auto app = std::make_shared<cr::app::Application>(ioService);
    app->start();

    std::atomic<std::uint32_t> result(0);
    app->startService<ClusterCalcService>("Thread1", "CalcService");
    app->startService<ClusterReqService>("Thread2", "ReqService", std::ref(result));

    ioService.run();

    BOOST_CHECK_EQUAL(result, 100);
}

BOOST_AUTO_TEST_SUITE_END()
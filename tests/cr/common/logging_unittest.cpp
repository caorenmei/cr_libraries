#include <boost/test/unit_test.hpp>

#include <sstream>

#include <boost/core/null_deleter.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/shared_ptr.hpp>

#include <cr/log/logging.h>

BOOST_AUTO_TEST_SUITE(logging)

struct LogSinkFixture
{
    using LogBackend = boost::log::sinks::text_ostream_backend;
    using LogFrontend = boost::log::sinks::synchronous_sink<LogBackend>;

    LogSinkFixture()
    {
        auto backend = boost::make_shared<LogBackend>();
        backend->auto_flush(true);
        backend->add_stream(boost::shared_ptr<std::ostream>(&sstrm, boost::null_deleter()));
        auto sink = boost::make_shared<LogFrontend>(backend);
        sink->set_formatter(
            boost::log::expressions::stream
            << "[" << boost::log::expressions::attr<const char*>("File") << ":" << boost::log::expressions::attr<int>("Line") << "]"
            << "\t<" << boost::log::expressions::attr<cr::SeverityLevel>("Severity") << ">"
            << "\t<" << boost::log::expressions::attr<const char*>("Tag") << ">"
            << "\t" << boost::log::expressions::message
        );
        this->sink = sink;
        boost::log::core::get()->add_sink(sink);
    }

    ~LogSinkFixture()
    {
        boost::log::core::get()->remove_sink(sink);
    }

    cr::Logger logger;
    std::stringstream sstrm;
    boost::shared_ptr<boost::log::sinks::sink> sink;
};

BOOST_FIXTURE_TEST_CASE(attributes, LogSinkFixture)
{
    CRLOG_DEBUG(logger, "cr.common") << "hello";
    std::string logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("logging_unittest.cpp"), std::string::npos);
    BOOST_CHECK_NE(logText.find("51"), std::string::npos);
    BOOST_CHECK_NE(logText.find("DEBUG"), std::string::npos);
    BOOST_CHECK_NE(logText.find("cr.common"), std::string::npos);
    BOOST_CHECK_NE(logText.find("hello"), std::string::npos);
}

BOOST_FIXTURE_TEST_CASE(severity, LogSinkFixture)
{
    std::string logText;

    sstrm.str("");
    CRLOG_TRACE(logger, "cr.common") << "hello";
    logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("TRACE"), std::string::npos);

    sstrm.str("");
    CRLOG_DEBUG(logger, "cr.common") << "hello";
    logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("DEBUG"), std::string::npos);

    sstrm.str("");
    CRLOG_INFO(logger, "cr.common") << "hello";
    logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("INFO"), std::string::npos);

    sstrm.str("");
    CRLOG_WARN(logger, "cr.common") << "hello";
    logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("WARN"), std::string::npos);

    sstrm.str("");
    CRLOG_ERROR(logger, "cr.common") << "hello";
    logText = sstrm.str();
    BOOST_CHECK_NE(logText.find("ERROR"), std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()


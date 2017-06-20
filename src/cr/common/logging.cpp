#include <cr/common/logging.h>

#include <boost/log/attributes/mutable_constant.hpp>

namespace cr
{
    Logger::Logger(SeverityLevel level/* = SeverityLevel::TRACE*/)
        : boost::log::sources::severity_logger_mt<SeverityLevel>(level)
    {
        add_attribute("File", boost::log::attributes::mutable_constant<const char*>(""));
        add_attribute("Line", boost::log::attributes::mutable_constant<int>(0));
        add_attribute("Tag", boost::log::attributes::mutable_constant<const char*>(""));
    }
}
#ifndef CR_COMMON_LOGGING_H_
#define CR_COMMON_LOGGING_H_

#include <cctype>
#include <cstddef>
#include <algorithm>
#include <array>
#include <iosfwd>
#include <iterator>

#include <boost/log/attributes.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

namespace cr
{
    namespace log
    {
        // 日志级别
        enum class SeverityLevel : std::size_t
        {
            // 追踪
            TRACE_LEVEL,
            // 调试
            DEBUG_LEVEL,
            // 信息
            INFO_LEVEL,
            // 警告
            WARN_LEVEL,
            // 错误
            ERROR_LEVEL,
        };

        // 输出日志级别
        template<typename CharT, typename TraitsT>
        inline std::basic_ostream<CharT, TraitsT >& operator<<(std::basic_ostream<CharT, TraitsT>& strm, SeverityLevel level)
        {
            static const char* const message[] =
            {
                "TRACE",
                "DEBUG",
                "INFO",
                "WARN",
                "ERROR",
            };
            std::size_t nlevel = static_cast<std::size_t>(level);
            if (nlevel < sizeof(message) / sizeof(message[0]))
            {
                strm << message[nlevel];
            }
            else
            {
                strm << nlevel;
            }
            return strm;
        }

        // 输出日志级别
        template<typename CharT, typename TraitsT>
        inline std::basic_istream<CharT, TraitsT >& operator>>(std::basic_istream<CharT, TraitsT>& strm, SeverityLevel& level)
        {
            level = SeverityLevel::TRACE_LEVEL;
            // 转换输入为大写
            std::basic_string<CharT, TraitsT> levelString;
            strm >> levelString;
            std::transform(levelString.begin(), levelString.end(), levelString.begin(), [](auto ch)
            {
                return std::toupper(ch);
            });
            // 匹配
            const std::array<std::string, 5> levels = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR" };
            auto iter = std::find_if(levels.begin(), levels.end(), [&](const std::string& s)
            {
                return std::equal(s.begin(), s.end(), levelString.begin(), levelString.end());
            });
            if (iter != levels.end())
            {
                level = static_cast<SeverityLevel>(std::distance(levels.begin(), iter));
            }
            return strm;
        }

        // 日志类型
        class Logger : public boost::log::sources::severity_channel_logger_mt<SeverityLevel, std::string>
        {
        public:

            // @param level 默认日志级别
            explicit Logger(const std::string& channel)
                : severity_channel_logger_mt(boost::log::keywords::channel = channel)
            {
                this->add_attribute("File", boost::log::attributes::mutable_constant<std::string>(""));
                this->add_attribute("Line", boost::log::attributes::mutable_constant<int>(0));
                this->add_attribute("Tag", boost::log::attributes::mutable_constant<std::string>(""));
            }

            // 设置属性值
            template <typename ValueType>
            ValueType setAttrValue(const char* name, ValueType value)
            {
                auto attr = boost::log::attribute_cast<boost::log::attributes::mutable_constant<ValueType>>(this->get_attributes()[name]);
                attr.set(value);
                return attr.get();
            }
        };
    }
}

// 日志宏 
#define CRLOG_SEV_IMPL(logger, level, file, line, tag) \
    BOOST_LOG_STREAM_WITH_PARAMS((logger),\
        (logger.setAttrValue("File", static_cast<std::string>(file))) \
        (logger.setAttrValue("Line", static_cast<int>(line))) \
        (logger.setAttrValue("Tag", static_cast<std::string>(tag))) \
        (boost::log::keywords::severity = (level)) \
    )

// 日志宏 
#define CRLOG_SEV(logger, level, tag) CRLOG_SEV_IMPL(logger, level, __FILE__, __LINE__, tag)

// 追踪级别日志 
#define CRLOG_TRACE(logger, tag) CRLOG_SEV(logger, cr::log::SeverityLevel::TRACE_LEVEL, tag)

// 调试级别日志 
#define CRLOG_DEBUG(logger, tag) CRLOG_SEV(logger, cr::log::SeverityLevel::DEBUG_LEVEL, tag)

// 信息级别日志 
#define CRLOG_INFO(logger, tag) CRLOG_SEV(logger, cr::log::SeverityLevel::INFO_LEVEL, tag)

// 警告级别日志 
#define CRLOG_WARN(logger, tag) CRLOG_SEV(logger, cr::log::SeverityLevel::WARN_LEVEL, tag)

// 错误级别日志 
#define CRLOG_ERROR(logger, tag) CRLOG_SEV(logger, cr::log::SeverityLevel::ERROR_LEVEL, tag)

#endif
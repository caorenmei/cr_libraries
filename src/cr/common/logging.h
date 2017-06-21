#ifndef CR_COMMON_LOGGING_H_
#define CR_COMMON_LOGGING_H_

#include <cstddef>
#include <iosfwd>
#include <iterator>

#include <boost/log/attributes.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace cr
{
    /** 日志级别 */ 
    enum class SeverityLevel : std::size_t
    {
        /** 追踪 */
        TRACE,
        /** 调试 */
        DEBUG,
        /** 信息 */
        INFO,
        /** 警告 */
        WARN,
        /** 错误 */
        ERROR,
    };

    /**
     * 输出日志级别
     * @param strm 格式化流
     * @param level 日志级别
     * @return 格式化流
     */
    template<typename CharT, typename TraitsT>
    inline std::basic_ostream<CharT, TraitsT >& operator<<(std::basic_ostream<CharT, TraitsT>& strm, cr::SeverityLevel level)
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

    /** 日志类型 */
    template <template<typename> class SeverityLogger>
    class BasicLogger : public SeverityLogger<SeverityLevel>
    {
    public:

        /**
         * 构造函数
         * @param level 默认日志级别
         */
        explicit BasicLogger(SeverityLevel level = SeverityLevel::TRACE)
            : SeverityLogger<SeverityLevel>(level)
        {
            this->add_attribute("File", boost::log::attributes::mutable_constant<const char*>(""));
            this->add_attribute("Line", boost::log::attributes::mutable_constant<int>(0));
            this->add_attribute("Tag", boost::log::attributes::mutable_constant<const char*>(""));
        }

        /**
         * 设置属性值
         * @param name 属性名
         * @param value 属性值
         * @return 属性值
         */
        template <typename ValueType>
        ValueType setAttrValue(const char* name, ValueType value)
        {
            auto attr = boost::log::attribute_cast<boost::log::attributes::mutable_constant<ValueType>>(this->get_attributes()[name]);
            attr.set(value);
            return attr.get();
        }
    };

    /** 非线程安全的日志 */
    using Logger = BasicLogger<boost::log::sources::severity_logger>;

    /** 非线程安全的日志 */
    using ThreadSafeLogger = BasicLogger<boost::log::sources::severity_logger_mt>;
}

/**
 * 日志宏 
 * @param logger 日志对象
 * @aram level 日志级别
 * @param tag Tag
 */
#define CRLOG_SEV(logger, level, tag) \
    BOOST_LOG_STREAM_WITH_PARAMS((logger),\
        (logger.setAttrValue("File", static_cast<const char*>(__FILE__))) \
        (logger.setAttrValue("Line", static_cast<int>(__LINE__))) \
        (logger.setAttrValue("Tag", static_cast<const char*>(tag))) \
        (boost::log::keywords::severity = (level)) \
    )

/**
 * 追踪级别日志 
 * @param logger 日志对象
 * @param tag Tag
 */
#define CRLOG_TRACE(logger, tag) CRLOG_SEV(logger, cr::SeverityLevel::TRACE, tag)

/**
 * 调试级别日志 
 * @param logger 日志对象
 * @param tag Tag
 */
#define CRLOG_DEBUG(logger, tag) CRLOG_SEV(logger, cr::SeverityLevel::DEBUG, tag)

/**
 * 信息级别日志 
 * @param logger 日志对象
 * @param tag Tag
 */
#define CRLOG_INFO(logger, tag) CRLOG_SEV(logger, cr::SeverityLevel::INFO, tag)

/**
 * 警告级别日志 
 * @param logger 日志对象
 * @param tag Tag
 */
#define CRLOG_WARN(logger, tag) CRLOG_SEV(logger, cr::SeverityLevel::WARN, tag)

/**
 * 错误级别日志 
 * @param logger 日志对象
 * @param tag Tag
 */
#define CRLOG_ERROR(logger, tag) CRLOG_SEV(logger, cr::SeverityLevel::ERROR, tag)

#endif
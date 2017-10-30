#ifndef COMMON_LUA_LOG_PARAMS_H_
#define COMMON_LUA_LOG_PARAMS_H_

#include <functional>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include <lua.hpp>
#include <selene/ExceptionTypes.h>
#include <selene/ResourceHandler.h>
#include <selene/primitives.h>

namespace cr
{
    namespace lua
    {
        // 日志参数
        class LogParams
        {
        public:

            LogParams();

            LogParams(std::string file, int line, std::string text);

            const std::string& getFile() const;

            int getLine() const;

            const std::string& getText() const;

        private:

            std::string file_;
            int line_;
            std::string text_;
        };

        // 带堆栈的日志参数
        class TracebackLogParams : public LogParams
        {
        public:

            TracebackLogParams();

            TracebackLogParams(std::string file, int line, std::string text, std::string traceback);

            const std::string& getTraceback() const;

        private:

            std::string traceback_;
        };

    }
}

namespace sel
{
    namespace detail
    {
        template <>
        struct is_primitive<cr::lua::LogParams> : std::integral_constant<bool, true>
        {};

        cr::lua::LogParams _get(_id<cr::lua::LogParams>, lua_State *l, const int index);

        cr::lua::LogParams _check_get(_id<cr::lua::LogParams>, lua_State *l, const int index);

        template <>
        struct is_primitive<cr::lua::TracebackLogParams> : std::integral_constant<bool, true>
        {};

        cr::lua::TracebackLogParams _get(_id<cr::lua::TracebackLogParams>, lua_State *l, const int index);

        cr::lua::TracebackLogParams _check_get(_id<cr::lua::TracebackLogParams>, lua_State *l, const int index);

    }
}

#endif
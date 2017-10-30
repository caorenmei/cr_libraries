#include "log_params.h"

#include "string.h"

namespace cr
{
    namespace lua
    {
        LogParams::LogParams()
            : LogParams("", 0, "")
        {}

        LogParams::LogParams(std::string file, int line, std::string text)
            : file_(std::move(file)),
            line_(line),
            text_(std::move(text))
        {}

        const std::string& LogParams::getFile() const
        {
            return file_;
        }

        int LogParams::getLine() const
        {
            return line_;
        }

        const std::string& LogParams::getText() const
        {
            return text_;
        }

        TracebackLogParams::TracebackLogParams()
            : TracebackLogParams("", 0, "", {})
        {}

        TracebackLogParams::TracebackLogParams(std::string file, int line, std::string text, std::string traceback)
            : LogParams(std::move(file), line, std::move(text)),
            traceback_(std::move(traceback))
        {}

        const std::string& TracebackLogParams::getTraceback() const
        {
            return traceback_;
        }

    }
}

namespace sel
{
    namespace detail
    {
        cr::lua::LogParams _get(_id<cr::lua::LogParams>, lua_State *l, const int index)
        {
            lua_Debug ar = {};
            lua_getstack(l, 1, &ar);
            lua_getinfo(l, "nSl", &ar);
            std::string text;
            int top = lua_gettop(l);
            for (int i = index; i <= top; ++i)
            {
                cr::lua::to_string(text, l, i);
            }
            return { ar.short_src, ar.currentline, std::move(text) };
        }

        cr::lua::LogParams _check_get(_id<cr::lua::LogParams>, lua_State *l, const int index)
        {
            return _get(_id<cr::lua::LogParams>(), l, index);
        }

        cr::lua::TracebackLogParams _get(_id<cr::lua::TracebackLogParams>, lua_State *l, const int index)
        {
            lua_Debug ar = {};
            lua_getstack(l, 1, &ar);
            lua_getinfo(l, "nSl", &ar);
            std::string text;
            int top = lua_gettop(l);
            for (int i = index; i <= top; ++i)
            {
                cr::lua::to_string(text, l, i);
            }
            luaL_traceback(l, l, nullptr, 1);
            const char* s = lua_tostring(l, -1);
            std::string traceback(s ? s : "");
            return { ar.short_src, ar.currentline, std::move(text), std::move(traceback) };
        }

        cr::lua::TracebackLogParams _check_get(_id<cr::lua::TracebackLogParams>, lua_State *l, const int index)
        {
            return _get(_id<cr::lua::TracebackLogParams>(), l, index);
        }
    }
}
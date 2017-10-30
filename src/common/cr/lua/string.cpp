#include "string.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>

namespace cr
{
    namespace lua
    {

        std::string& to_string(std::string& buffer, lua_State* l, const int index, int depth/* = 0*/)
        {
            char temp[64];
            auto type = lua_type(l, index);
            switch (type)
            {
            case LUA_TNIL:
            {
                buffer.append("nil");
                break;
            }
            case LUA_TBOOLEAN:
            {
                auto value = lua_toboolean(l, index);
                buffer.append(value ? "true" : "false");
                break;
            }
            case LUA_TLIGHTUSERDATA:
            {
                auto value = lua_touserdata(l, index);
                std::snprintf(temp, sizeof(temp), "0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(value));
                buffer.append(temp);
                break;
            }
            case LUA_TNUMBER:
            {
                if (lua_isinteger(l, index))
                {
                    auto value = lua_tointeger(l, index);
                    std::snprintf(temp, sizeof(temp), "%" PRId64, static_cast<std::int64_t>(value));
                }
                else
                {
                    auto value = lua_tonumber(l, index);
                    std::snprintf(temp, sizeof(temp), "%g", static_cast<double>(value));
                }
                buffer.append(temp);
                break;
            }
            case LUA_TSTRING:
            {
                std::size_t size = 0;
                auto value = lua_tolstring(l, index, &size);
                buffer.append(value, size);
                break;
            }
            case LUA_TTABLE:
            {
                buffer.append("{\n");
                lua_pushnil(l);
                auto tableIndex = index > 0 ? index : index - 1;
                while (lua_next(l, tableIndex))
                {
                    buffer.append((depth + 1) * 2, ' ');
                    to_string(buffer, l, -2, depth + 1);
                    buffer.append(": ");
                    to_string(buffer, l, -1, depth + 1);
                    buffer.append(", \n");
                    lua_pop(l, 1);
                }
                buffer.append(depth * 2, ' ');
                buffer.append("}");
                break;
            }
            case LUA_TFUNCTION:
            {
                auto value = lua_tocfunction(l, index);
                std::snprintf(temp, sizeof(temp), "function 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(value));
                buffer.append(temp);
                break;
            }
            case LUA_TUSERDATA:
            {
                auto value = lua_touserdata(l, index);
                std::snprintf(temp, sizeof(temp), "0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(value));
                buffer.append(temp);
                break;
            }
            case LUA_TTHREAD:
            {
                auto value = lua_tothread(l, index);
                std::snprintf(temp, sizeof(temp), "thread 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(value));
                buffer.append(temp);
                break;
            }
            default:
                break;
            }

            return buffer;
        }

        std::string to_string(lua_State* l, int index, int depth/* = 0*/)
        {
            std::string buffer;
            to_string(buffer, l, index, depth);
            return buffer;
        }
    }
}
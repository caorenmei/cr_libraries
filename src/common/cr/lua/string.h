#ifndef COMMON_LUA_STRING_H_
#define COMMON_LUA_STRING_H_

#include <string>
#include <typeinfo>

#include <lua.hpp>

namespace cr
{
    namespace lua
    {

        std::string& to_string(std::string& buf, lua_State* l, int index, int depth = 0);

        std::string to_string(lua_State* l, int index, int depth = 0);
    }
}

#endif

#ifndef COMMON_LUA_CHECK_TYPE_H_
#define COMMON_LUA_CHECK_TYPE_H_

#include <selene/traits.h>

namespace sel
{
    namespace detail
    {
        template <typename T>
        inline bool _check_type(_id<T>, lua_State* l, const int index)
        {
            return true;
        }

        inline bool _check_type(_id<int>, lua_State* l, const int index)
        {
            return lua_isnumber(l, index);
        }

        inline bool _check_type(_id<unsigned int>, lua_State* l, const int index)
        {
            return lua_isnumber(l, index);
        }

        inline bool _check_type(_id<long long>, lua_State* l, const int index)
        {
            return lua_isnumber(l, index);
        }

        inline bool _check_type(_id<bool>, lua_State* l, const int index)
        {
            return lua_isboolean(l, index);
        }

        inline bool _check_type(_id<lua_Number>, lua_State* l, const int index)
        {
            return lua_isnumber(l, index);
        }

        inline bool _check_type(_id<std::string>, lua_State* l, const int index)
        {
            return lua_isstring(l, index);
        }
    }
}

#endif

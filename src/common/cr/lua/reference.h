#ifndef COMMON_LUA_REFERENCE_H_
#define COMMON_LUA_REFERENCE_H_

#include <functional>
#include <typeinfo>
#include <utility>

#include <lua.hpp>
#include <selene/ExceptionTypes.h>
#include <selene/ResourceHandler.h>
#include <selene/primitives.h>

namespace sel
{
    namespace detail
    {
        template <typename T>
        struct is_primitive<std::reference_wrapper<T>> : std::integral_constant<bool, true>
        {};

        template <typename T>
        std::reference_wrapper<T> _get(_id<std::reference_wrapper<T>>, lua_State *l, const int index)
        {
            ResetStackOnScopeExit savedStack(l);
            lua_pushvalue(l, index);

            void* p = nullptr;
            if (lua_type(l, -1) == LUA_TLIGHTUSERDATA)
            {
                p = lua_touserdata(l, -1);
            }

            return { *static_cast<T*>(p) };
        }

        template <typename T>
        std::reference_wrapper<T> _check_get(_id<std::reference_wrapper<T>>, lua_State *l, const int index)
        {
            return _get(_id<std::reference_wrapper<T>>(), l, index);
        }

        template <typename T>
        void _push(lua_State *l, const std::reference_wrapper<T>& ref)
        {
            lua_pushlightuserdata(l, &ref.get());
        }
    }
}

#endif
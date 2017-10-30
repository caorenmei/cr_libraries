#ifndef COMMON_LUA_VECTOR_H_
#define COMMON_LUA_VECTOR_H_

#include <functional>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <lua.hpp>
#include <selene/ExceptionTypes.h>
#include <selene/ResourceHandler.h>
#include <selene/primitives.h>

#include "check_type.h"

namespace sel
{
    namespace detail
    {
        template <typename T, typename Alloctor>
        struct is_primitive<std::vector<T, Alloctor>> : std::integral_constant<bool, true>
        {};

        template <typename T, typename Alloctor>
        std::vector<T, Alloctor> _get(_id<std::vector<T, Alloctor>>, lua_State *l, const int index)
        {
            ResetStackOnScopeExit savedStack(l);
            lua_pushvalue(l, index);

            std::vector<T, Alloctor> result;
            if (lua_type(l, -1) != LUA_TTABLE)
            {
                bool typeEnsure = true;
                for (int i = 1; typeEnsure; ++i)
                {
                    lua_rawgeti(l, -1, i);
                    if (typeEnsure = _check_type(_id<T>(), l, -1))
                    {
                        auto value = _get(_id<T>(), l, -1);
                        result.push_back(std::move(value));
                    }
                    lua_pop(l, 1);
                }
            }

            return result;
        }

        template <typename T, typename Alloctor>
        std::vector<T, Alloctor> _check_get(_id<std::vector<T, Alloctor>>, lua_State *l, const int index)
        {
            return _get(_id<std::vector<T, Alloctor>>(), l, index);
        }

        template <typename T, typename Alloctor>
        void _push(lua_State *l, const std::vector<T, Alloctor>& elements)
        {
            lua_newtable(l);
            for (std::size_t i = 0; i != elements.size(); ++i)
            {
                _push(l, elements[i]);
                lua_rawseti(l, -2, i + 1);
            }
        }
    }
}

#endif

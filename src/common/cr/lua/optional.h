#ifndef COMMON_LUA_OPTIONAL_H_
#define COMMON_LUA_OPTIONAL_H_

#include <functional>
#include <type_traits>
#include <typeinfo>

#include <boost/optional.hpp>
#include <lua.hpp>
#include <selene/ExceptionTypes.h>
#include <selene/ResourceHandler.h>
#include <selene/primitives.h>

#include "check_type.h"

namespace sel
{
    namespace detail
    {
        template <typename T>
        struct is_primitive<boost::optional<T>> : std::integral_constant<bool, true>
        {};

        template <typename T>
        boost::optional<T> _get(_id<boost::optional<T>>, lua_State *l, const int index)
        {
            ResetStackOnScopeExit savedStack(l);
            lua_pushvalue(l, index);

            boost::optional<T> result;
            if (_check_type(_id<T>(), l, -1))
            {
                result = _get(_id<T>(), l, -1);
            }

            return result;
        }

        template <typename T>
        boost::optional<T> _check_get(_id<boost::optional<T>>, lua_State *l, const int index)
        {
            return _get(_id<T>(), l, index);
        }

        template <typename T>
        void _push(lua_State *l, const boost::optional<T> & element)
        {
            if (element)
            {
                _push(l, *element);
            }
            else
            {
                lua_pushnil(l);
            }
        }
    }
}

#endif
#ifndef CR_COMMON_LUA_PROTOBUF_H_
#define CR_COMMON_LUA_PROTOBUF_H_

#include <functional>
#include <type_traits>
#include <typeinfo>

#include <boost/optional.hpp>
#include <google/protobuf/message.h>
#include <lua.hpp>
#include <selene/ExceptionTypes.h>
#include <selene/ResourceHandler.h>
#include <selene/traits.h>

namespace sel
{
    namespace detail
    {
        std::shared_ptr<google::protobuf::Message> _get(_id<std::shared_ptr<google::protobuf::Message>>, lua_State *l, const int index);

        std::shared_ptr<google::protobuf::Message> _check_get(_id<std::shared_ptr<google::protobuf::Message>>, lua_State *l, const int index);

        void _push(lua_State *l, const std::shared_ptr<google::protobuf::Message>& message);
    }
}

#include <selene/primitives.h>

namespace sel
{
    namespace detail
    {
        template <>
        struct is_primitive<std::shared_ptr<google::protobuf::Message>> : std::integral_constant<bool, true>
        {};
    }
}

#endif

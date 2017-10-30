#include "protobuf.h"

#include <cr/protobuf/reflect.h>

#include "protobuf_pool.h"

namespace sel
{
    namespace detail
    {
        std::shared_ptr<google::protobuf::Message> _get(_id<std::shared_ptr<google::protobuf::Message>>, lua_State *l, const int index)
        {
            ResetStackOnScopeExit savedStack(l);
            if (lua_type(l, index) != LUA_TTABLE)
            {
                return nullptr;
            }
            // name 
            lua_pushvalue(l, index);
            lua_pushstring(l, "name");
            lua_rawget(l, -2);
            if (lua_type(l, -1) != LUA_TSTRING)
            {
                return nullptr;
            }
            std::string typeName  = lua_tostring(l, -1);
            lua_pop(l, 2);
            // message
            std::shared_ptr<google::protobuf::Message> message(cr::protobuf::getMessageFromName(typeName));
            if (message == nullptr)
            {
                return nullptr;
            }
            // protobuf pool
            auto protobuf = cr::lua::ProtobufPool::pop(l);
            if (protobuf == nullptr)
            {
                return nullptr;
            }
            // parse
            lua_pushstring(l, "msg");
            lua_rawget(l, -2);
            if (lua_type(l, -1) != LUA_TTABLE)
            {
                return nullptr;
            }
            protobuf->lua2pb(l, -1, message.get());
            return message;
        }

        std::shared_ptr<google::protobuf::Message> _check_get(_id<std::shared_ptr<google::protobuf::Message>>, lua_State *l, const int index)
        {
            auto message = _get(_id<std::shared_ptr<google::protobuf::Message>>(), l, index);
            if (message == nullptr)
            {
                std::string typeName = "google.protobuf.Message";
                ResetStackOnScopeExit savedStack(l);
                if (lua_type(l, index) == LUA_TTABLE)
                {
                    lua_pushvalue(l, index);
                    lua_pushstring(l, "name");
                    lua_rawget(l, -2);
                    if (lua_type(l, -1) == LUA_TSTRING)
                    {
                        typeName = lua_tostring(l, -1);
                    }
                }
                throw GetUserdataParameterFromLuaTypeError{ typeName,index };
            }
            return message;
        }

        void _push(lua_State *l, const std::shared_ptr<google::protobuf::Message>& message)
        {
            auto protobuf = cr::lua::ProtobufPool::pop(l);
            if (protobuf == nullptr)
            {
                throw CopyUnregisteredType(typeid(*message));
            } 
            lua_newtable(l);
            // name
            std::string typeName = message->GetTypeName();
            lua_pushstring(l, "name");
            lua_pushlstring(l, typeName.c_str(), typeName.size());
            lua_rawset(l, -3);
            // message
            lua_pushstring(l, "msg");
            if (!protobuf->pb2lua(l, *message))
            {
                lua_pop(l, 1);
                throw CopyUnregisteredType(typeid(*message));
            }
            lua_rawset(l, -3);
        }
    }
}
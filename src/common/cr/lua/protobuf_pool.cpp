#include "protobuf_pool.h"

#include <cr/protobuf/reflect.h>

#include <selene/ResourceHandler.h>

namespace cr
{
    namespace lua
    {

        int ProtobufPool::id = 0;

        void ProtobufPool::addPb2LuaHandler(const google::protobuf::Descriptor* descriptor, Pb2LuaHandler handler)
        {
            serializes_[descriptor] = std::move(handler);
        }

        void ProtobufPool::addLua2PbHandler(const google::protobuf::Descriptor* descriptor, Lua2PbHandler handler)
        {
            deserializes_[descriptor] = std::move(handler);
        }

        bool ProtobufPool::pb2lua(lua_State* l, const google::protobuf::Message& message) const
        {
            auto iter = serializes_.find(message.GetDescriptor());
            if (iter != serializes_.end())
            {
                iter->second(l, message);
                return true;
            }
            return false;
        }

        bool ProtobufPool::lua2pb(lua_State* l, int index, google::protobuf::Message* message) const
        {
            auto iter = deserializes_.find(message->GetDescriptor());
            if (iter != deserializes_.end())
            {
                iter->second(l, index, message);
                return true;
            }
            return false;
        }

        boost::optional<std::string> ProtobufPool::encode(std::shared_ptr<google::protobuf::Message> message)
        {
            if (message != nullptr)
            {
                return message->SerializeAsString();
            }
            return {};
        }

        boost::optional<std::shared_ptr<google::protobuf::Message>> ProtobufPool::decode(std::string name, std::string body)
        {
            std::shared_ptr<google::protobuf::Message> message(cr::protobuf::getMessageFromName(name));
            if (message != nullptr && message->ParseFromString(body))
            {
                return message;
            }
            return {};
        }

        void  ProtobufPool::push(ProtobufPool*  protobuf, lua_State* l)
        {
            sel::ResetStackOnScopeExit savedStack(l);
            lua_pushlightuserdata(l, &cr::lua::ProtobufPool::id);
            lua_pushlightuserdata(l, protobuf);
            lua_settable(l, LUA_REGISTRYINDEX);
        }

        ProtobufPool* ProtobufPool::pop(lua_State* l)
        {
            sel::ResetStackOnScopeExit savedStack(l);
            lua_pushlightuserdata(l, &cr::lua::ProtobufPool::id);
            lua_gettable(l, LUA_REGISTRYINDEX);
            auto pool = lua_touserdata(l, -1);
            return static_cast<cr::lua::ProtobufPool*>(pool);
        }
    }
}
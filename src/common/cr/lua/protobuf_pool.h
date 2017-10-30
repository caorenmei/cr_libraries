#ifndef CR_COMMON_LUA_PROTOBUF_POOL_H_
#define CR_COMMON_LUA_PROTOBUF_POOL_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include <boost/optional.hpp>
#include <google/protobuf/message.h>
#include <lua.hpp>

namespace cr
{
    namespace lua
    {
        // pb序列化
        class ProtobufPool
        {
        public:

            // 类Id
            static int id;

            // pb 2 lua
            using Pb2LuaHandler = std::function<void(lua_State*, const google::protobuf::Message&)>;

            // lua 2 pb 
            using Lua2PbHandler = std::function<void(lua_State*, int, google::protobuf::Message*)>;

            // 注册序列化处理器
            void addPb2LuaHandler(const google::protobuf::Descriptor* descriptor, Pb2LuaHandler handler);

            // 注册反序列化处理器
            void addLua2PbHandler(const google::protobuf::Descriptor* descriptor, Lua2PbHandler handler);

            // 序列化消息
            bool pb2lua(lua_State* l, const google::protobuf::Message& message) const;

            // 反序列化消息
            bool lua2pb(lua_State* l, int index, google::protobuf::Message* message) const;

            // 编码消息
            boost::optional<std::string > encode(std::shared_ptr<google::protobuf::Message> message);

            // 解码消息
            boost::optional<std::shared_ptr<google::protobuf::Message>> decode(std::string name, std::string body);

            // 保存对象到lua
            static void push(ProtobufPool*  protobuf, lua_State* l);

            // 从lua取出对象
            static ProtobufPool* pop(lua_State* l);

        private:
            // 序列化
            std::map<const google::protobuf::Descriptor*, Pb2LuaHandler> serializes_;
            // 反序列化
            std::map<const google::protobuf::Descriptor*, Lua2PbHandler> deserializes_;
        };
    }
}

#endif

#include <cr/network/protobuf_utils.h>

namespace cr
{
    namespace network
    {
        google::protobuf::Message* getProtobufMessageFromName(const std::string& typeName)
        {
            google::protobuf::Message* message = nullptr;
            auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
            if (descriptor != nullptr)
            {
                auto prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
                if (prototype != nullptr)
                {
                    return prototype->New();
                }
            }
            return nullptr;
        }
    }
}
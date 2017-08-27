#include "reflect.h"

#include <google/protobuf/io/zero_copy_stream.h>

namespace cr
{
    namespace protobuf
    {
        google::protobuf::Message* getMessageFromName(const std::string& typeName)
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
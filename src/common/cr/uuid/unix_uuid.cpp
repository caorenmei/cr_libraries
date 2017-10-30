#if !defined(WIN32)

#include "uuid.h"

#include <cstring>

#include <uuid/uuid.h>

namespace cr
{
    namespace uuid
    {
        boost::uuids::uuid generate()
        {
            uuid_t ruuid;
            uuid_generate_time_safe(ruuid);
            boost::uuids::uuid uuid;
            std::memcpy(uuid.data, ruuid, sizeof(uuid.data));
            return uuid;
        }
    }
}

#endif
#if defined(WIN32)

#include "uuid.h"

#include <objbase.h>

#include <cstring>

namespace cr
{
    namespace uuid
    {
        boost::uuids::uuid generate()
        {
            GUID guid;
            CoCreateGuid(&guid);
            boost::uuids::uuid uuid;
            std::memcpy(uuid.data, &guid, sizeof(uuid.data));
            return uuid;
        }
    }
}

#endif
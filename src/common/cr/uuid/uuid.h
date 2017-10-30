#ifndef CR_COMMON_UUID_UUID_H_
#define CR_COMMON_UUID_UUID_H_

#include <boost/uuid/uuid.hpp>

namespace cr
{
    namespace uuid
    {
        /**
         * 生成UUID
         * @return uuid
         */
        boost::uuids::uuid generate();
    }
}

#endif
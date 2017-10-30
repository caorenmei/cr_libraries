#ifndef CR_COMMON_STREAMS_EMPTY_H_
#define CR_COMMON_STREAMS_EMPTY_H_

namespace cr
{
    template <typename T>
    auto empty()
    {
        return from(static_cast<T*>(0), static_cast<T*>(0));
    }
}

#endif

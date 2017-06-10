#ifndef CR_COMMON_TYPE_UTILS_H_
#define CR_COMMON_TYPE_UTILS_H_

#include <type_traits>

namespace cr
{
    // 值类型
    template <typename T>
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
}

#endif

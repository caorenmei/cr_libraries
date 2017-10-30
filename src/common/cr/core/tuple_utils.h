#ifndef CR_COMMON_TUPLE_UTILS_H_
#define CR_COMMON_TUPLE_UTILS_H_

#include <tuple>

namespace cr
{
    namespace tuple
    {

        template <typename TTuple>
        struct PopFront;

        template <typename T, typename... TArgs>
        struct PopFront<std::tuple<T, TArgs...>>
        {
            using type = std::tuple<TArgs...>;
        };

        template <typename TTuple>
        using PopFrontType = typename PopFront<TTuple>::type;

        /** 移除头部元素 */
        template <typename TTuple, std::size_t N>
        struct EraseFrontEements
        {
            using type = typename EraseFrontEements<PopFrontType<TTuple>, N - 1>::type;
        };

        /** 移除头部元素 */
        template <typename TTuple>
        struct EraseFrontEements<TTuple, 0>
        {
            using type = TTuple;
        };

        /** 移除头部元素 */
        template <typename TTuple, std::size_t N>
        using EraseFrontEementsType = typename EraseFrontEements<TTuple, N>::type;
    }
}

#endif

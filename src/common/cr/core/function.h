#ifndef CR_COMMON_FUNCTION_H_
#define CR_COMMON_FUNCTION_H_

#include <type_traits>
#include <utility>

namespace cr
{
    namespace fun
    {
        template <typename Functor, typename... Args>
        auto shiftAux(std::integral_constant<std::size_t, 0>, std::true_type, Functor&& call, Args&&... args)
        {
            return call(std::forward<Args>(args)...);
        }

        template <std::size_t N, typename Functor, typename T, typename... Args>
        auto shiftAux(std::integral_constant<std::size_t, N>, std::false_type, Functor&& call, T&& arg, Args&&... args)
        {
            return shiftAux(std::integral_constant<std::size_t, N - 1>(), std::integral_constant<bool, N - 1 == 0>(), std::forward<Functor>(call), std::forward<Args>(args)...);
        }

        /**
        * 移动函数参数
        * @param call 处理器.
        * @param args 参数列表.
        * @return call().
        */
        template <std::size_t N, typename Functor, typename... Args>
        auto shift(Functor&& call, Args&&... args)
        {
            return shiftAux(std::integral_constant<std::size_t, N>(), std::integral_constant<bool, N == 0>(), std::forward<Functor>(call), std::forward<Args>(args)...);
        }
    }
}

#endif

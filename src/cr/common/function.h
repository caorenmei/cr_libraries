#ifndef CR_COMMON_FUNCTION_H_
#define CR_COMMON_FUNCTION_H_

#include <type_traits>
#include <utility>

namespace cr
{
    namespace fun
    {
        template <typename Functor, typename... Args>
        auto shift(std::integral_constant<std::size_t, 0>, Functor& call, Args&&... args)
        {
            return call(std::forward<Args>(args)...);
        }

        /**
         * 移动函数参数
         * @param right 参数移动位数
         * @param call 处理器.
         * @param args 参数列表.
         * @return call().
         */
        template <std::size_t N, typename Functor, typename T, typename... Args>
        auto shift(std::integral_constant<std::size_t, N> right, Functor& call, T&& arg, Args&&... args)
        {
            return shift(std::integral_constant<std::size_t, N - 1>(), call, std::forward<Args>(args)...);
        }
    }
}

#endif

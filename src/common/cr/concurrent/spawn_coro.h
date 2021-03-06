﻿#ifndef CR_CONCRRENT_SPAWN_CORO_HPP_
#define CR_CONCRRENT_SPAWN_CORO_HPP_

#include <atomic>
#include <tuple>
#include <utility>

#include <boost/asio/async_result.hpp>

#include <cr/core/function.h>
#include <cr/core/tuple_utils.h>
#include <cr/core/type_utils.h>

namespace cr
{
    namespace concurrent
    {
        namespace coro
        {
            /** The asynchronous parameters. */
            template <typename... TArgs>
            class CallCapture
            {
            public:

                /** 捕获参数个数. */
                enum : std::size_t { capture_size = sizeof...(TArgs), };
                
                /**
                 * Constructor./
                 * @param coro spawn创建的协程
                 * @param params 绑定的参数
                 */
                CallCapture(Coroutine coro, ValueType<TArgs>&... params)
                    : coro_(std::move(coro)),
                    params_(params...)
                {}

                /**
                 * Function call operator.
                 * @param params 回调结果参数.
                 */
                void operator()(ValueType<TArgs>... params, ...)
                {
                    params_ = std::forward_as_tuple(std::move(params)...);
                }

                /** 唤醒协程. */
                void resume()
                {
                    coro_.resume();
                }

                /** 让出控制权. */
                void yield()
                {
                    coro_.yield();
                }

            private:

                // 协程
                Coroutine coro_;
                // 绑定的参数
                std::tuple<ValueType<TArgs>&...> params_;
            };

            /**
             * 生成一个异步任务.
             * @param coro spawn创建的协程.
             * @param params 绑定的参数.
             * @return 异步任务;
             */
            template <typename... TArgs>
            auto async(Coroutine coro, TArgs&... params)
            {
                return CallCapture<TArgs...>{ coro, params... };
            }
            /** 工具类，返回值类型计算. */
            template <typename CallCapture, typename... TArgs>
            struct CallHelper
            {
                // 结果列表
                using ValuesType = std::tuple<ValueType<TArgs>...>;

                // 返回类型
                using ResultsType = cr::tuple::EraseFrontEementsType<ValuesType, CallCapture::capture_size>;
            };

            /** 异步调用. */
            template <typename CallCapture, typename... TArgs>
            struct CallHandler
            {
                // 返回类型
                using ResultsType = typename CallHelper<CallCapture, TArgs...>::ResultsType;

                /**
                 * Constructor.
                 * @param capture 参数捕获器.
                 */
                explicit CallHandler(CallCapture capture)
                    : capature(std::move(capture)),
                    results(nullptr),
                    ready(nullptr)
                {}

                /**
                 * 为捕获的参数赋值.
                 *
                 * @param args 回调的参数.
                 */
                template <typename... UArgs>
                auto operator()(UArgs&&... args) -> std::enable_if_t<sizeof...(UArgs) == sizeof...(TArgs)>
                {
                    capature(std::forward<UArgs>(args)...);
                    cr::fun::shift<CallCapture::capture_size>([this](auto&&... params)
                    {
                        *results = std::forward_as_tuple(params...);
                    }, std::forward<UArgs>(args)...);
                    if (--*ready == 0)
                    {
                        capature.resume();
                    }
                }

                // 异步参数捕获器
                CallCapture capature;
                // 返回值
                ResultsType* results;
                // 计数器
                std::atomic<std::size_t>* ready;
            };

            /** 异步调用. */
            template <typename CallCapture, typename... TArgs>
            struct CallResult
            {
                /** 异步调用返回值类型 */
                using type = typename CallHelper<CallCapture, TArgs...>::ResultsType;
                /**
                 * Constructor.
                 * @param handler 包装的回调.
                 */
                explicit CallResult(CallHandler<CallCapture, TArgs...>& handler)
                    : handler(handler),
                    ready(2)
                {
                    handler.ready = &ready;
                    handler.results = &results;
                }

                /**
                 * 获取回调返回值.
                 * @return 回调返回值.
                 */
                type get()
                {
                    if (--ready == 1)
                    {
                        handler.capature.yield();
                    }
                    return std::move(results);
                }

            private:
                    
                // 异步回调
                CallHandler<CallCapture, TArgs...> handler;
                // 状态
                std::atomic<std::size_t> ready;
                // 返回结果
                type results;
            };

            /** 计算回调类型. */
            template <typename Capture, typename Signature>
            struct AsioHandlerType;

            /** 计算回调类型. */
            template <typename Capture, typename ReturnType, typename... TArgs>
            struct AsioHandlerType<Capture, ReturnType(TArgs...)>
            {
                using type = CallHandler<Capture, TArgs...>;
            };
        }
    }
}

namespace boost
{
    namespace asio
    {
        template <typename Signature, typename... TArgs>
        struct handler_type<cr::concurrent::coro::CallCapture<TArgs...>, Signature>
        {
            using type = typename cr::concurrent::coro::AsioHandlerType<
                cr::concurrent::coro::CallCapture<TArgs...>, Signature>::type;
        };

        template <typename Capture, typename... TArgs>
        struct async_result<cr::concurrent::coro::CallHandler<Capture, TArgs...>>
            : cr::concurrent::coro::CallResult<Capture, TArgs...>
        {
            using cr::concurrent::coro::CallResult<Capture, TArgs...>::CallResult;
        };
    }
}

#endif

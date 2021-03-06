﻿#ifndef CR_COMMON_STREAMS_GENERATE_H_
#define CR_COMMON_STREAMS_GENERATE_H_

#include <type_traits>

#include <boost/optional.hpp>

namespace cr
{

    namespace streams
    {
        /** 生成迭代器. */
        template <typename TGenerator>
        class GenerateEnumerator
        {
        public:
            typedef std::conditional_t<std::is_function<TGenerator>::value, std::add_pointer_t<TGenerator>, TGenerator> FuncType;
            typedef std::result_of_t<FuncType()> ReferenceType;
            typedef std::remove_reference_t<ReferenceType> ValueType;

            GenerateEnumerator(TGenerator generator)
                : generator_(std::move(generator))
            {}

            bool next()
            {
                current_ = generator_();
                return true;
            }

            ReferenceType current()
            {
                return *current_;
            }

        private:
            TGenerator generator_;
            boost::optional<ValueType> current_;
        };
    }

    template <typename TGenerator>
    auto generate(TGenerator gen)
    {
        auto enumerator = streams::GenerateEnumerator<TGenerator>{ gen };
        return Stream<decltype(enumerator)>{ enumerator };
    }

    template <typename TArithmetic, typename = std::enable_if_t<std::is_arithmetic<TArithmetic>::value>>
    auto generate(TArithmetic initValue, TArithmetic increValue)
    {
        std::size_t genCount = 0;
        return generate([=]() mutable
        {
            if (genCount++ > 0)
            {
                initValue += increValue;
            }
            return initValue;
        });
    }
}

#endif

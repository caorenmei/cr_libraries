#ifndef CR_COMMON_STREAMS_H_
#define CR_COMMON_STREAMS_H_

#include <iterator>
#include <utility>

#include <boost/optional.hpp>

#include "streams_enumerators.h"

namespace cr
{
    template <typename TEnumerable>
    class Stream
    {
    public:
        typedef TEnumerable EnumeratorType;
        typedef typename TEnumerable::ValueType ValueType;
        typedef typename TEnumerable::ValueType ReferenceType;

        explicit Stream(TEnumerable enumerator)
            : enumerator_(std::move(enumerator))
        {}

        ~Stream()
        {}

        const TEnumerable& getEnumerator() const
        {
            return enumerator_;
        }

        TEnumerable& getEnumerator()
        {
            return enumerator_;
        }

        template <typename TAction>
        void forEach(TAction action)
        {
            while (enumerator_.next())
            {
                action(enumerator_.current());
            }
        }

        template <typename TPredicate>
        auto filter(TPredicate predicate)
        {
            auto enumerator = streams::FilterEnumerator<TEnumerable, TPredicate>{ std::move(enumerator_), std::move(predicate) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto deepClone()
        {
            auto enumerator = streams::DeepCloneEnumerator<TEnumerable>{ std::move(enumerator_) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        template <typename TMapper>
        auto map(TMapper mapper)
        {
            auto enumerator = streams::MapperEnumerator<TEnumerable, TMapper>{ std::move(enumerator_), std::move(mapper) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        template <typename TMapper>
        auto flatMap(TMapper mapper)
        {
            auto mapperEnumerator = streams::MapperEnumerator<TEnumerable, TMapper>{ std::move(enumerator_), std::move(mapper) };
            auto flatMapperEnumerator = streams::FlatEnumerator<decltype(mapperEnumerator)>{ std::move(mapperEnumerator) };
            return Stream<decltype(flatMapperEnumerator)>{ flatMapperEnumerator };
        }

        template <typename TInitValueType, typename TAccumulator>
        auto reduce(TInitValueType initValue, TAccumulator accumulator)
        {
            forEach([&](auto&& a)
            {
                initValue = accumulator(initValue, a);
            });
            return initValue;
        }

        template <typename TAccumulator>
        auto reduce(TAccumulator accumulator)
        {
            boost::optional<ValueType> initValue;
            forEach([&](auto&& a)
            {
                if (initValue)
                {
                    initValue = accumulator(*initValue, a);
                }
                else
                {
                    initValue = a;
                }
            });
            return initValue;
        }

        template <typename UEnumerable>
        auto concat(Stream<UEnumerable> otherStream)
        {
            auto enumerator = streams::ConcatEnumerator<TEnumerable, UEnumerable>{ std::move(enumerator_), std::move(otherStream.getEnumerator()) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        template <typename TPredicate>
        auto distinct(TPredicate predicate)
        {
            auto enumerator = streams::DistinctEnumerator<TEnumerable, TPredicate>{ std::move(enumerator_), std::move(predicate) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto distinct()
        {
            return distinct([](auto&& a, auto&& b) { return a == b; });
        }

        template <typename TComparator>
        auto sorted(TComparator comparator)
        {
            auto enumerator = streams::SortedEnumerator<TEnumerable, TComparator>{ std::move(enumerator_), std::move(comparator) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto sorted()
        {
            return sorted([](auto&& lhs, auto&& rhs) { return lhs < rhs; });
        }

        template <typename UEnumerable, typename TEqualTo>
        bool equals(Stream<UEnumerable> otherStream, TEqualTo equalTo)
        {
            auto rightEnumerator = std::move(otherStream.getEnumerator());
            bool leftHasNext = enumerator_.next();
            bool rightHasNext = rightEnumerator.next();
            while (leftHasNext && rightHasNext)
            {
                if (!equalTo(enumerator_.current(), rightEnumerator.current()))
                {
                    return false;
                }
                leftHasNext = enumerator_.next();
                rightHasNext = rightEnumerator.next();
            }
            return !leftHasNext && !rightHasNext;
        }

        template <typename UEnumerable>
        bool equals(Stream<UEnumerable> otherStream)
        {
            return equals(std::move(otherStream), [](auto&& a, auto&& b) { return a == b; });
        }

        auto reverse() const
        {
            auto enumerator = streams::ReverseEnumerator<TEnumerable>{ std::move(enumerator_) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto findFirst()
        {
            boost::optional<ValueType> firstValue;
            if (enumerator_.next())
            {
                firstValue = enumerator_.current();
            }
            return firstValue;
        }

        auto findLast()
        {
            boost::optional<ValueType> lastValue;
            while (enumerator_.next())
            {
                lastValue = enumerator_.current();
            }
            return lastValue;
        }

        template <typename TAction>
        auto peek(TAction action)
        {
            auto enumerator = streams::PeekEnumerator<TEnumerable, TAction>{ std::move(enumerator_), std::move(action) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto limit(std::size_t n)
        {
            auto enumerator = streams::LimitEnumerator<TEnumerable>{ std::move(enumerator_), n };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        auto skip(std::size_t n)
        {
            auto enumerator = streams::SkipEnumerator<TEnumerable>{ std::move(enumerator_), n };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        template <typename TPredicate>
        auto until(TPredicate predicate)
        {
            auto enumerator = streams::UntilEnumerator<TEnumerable, TPredicate>{ std::move(enumerator_), std::move(predicate) };
            return Stream<decltype(enumerator)>{ enumerator };
        }

        template <typename TCollection>
        TCollection toList()
        {
            TCollection collection;
            forEach([&](auto&& e)
            {
                collection.push_back(e);
            });
            return collection;
        }

        template <typename TComparator>
        auto min(TComparator comparator)
        {
            boost::optional<ValueType> minValue;
            forEach([&](auto&& a)
            {
                if (!minValue)
                {
                    minValue = a;
                }
                else if (!comparator(*minValue, a))
                {
                    minValue = a;
                }
            });
            return minValue;
        }

        auto min()
        {
            return min([](auto&& lhs, auto&& rhs) { return lhs < rhs; });
        }

        template <typename TComparator>
        auto max(TComparator comparator)
        {
            boost::optional<ValueType> maxValue;
            forEach([&](auto&& a)
            {
                if (!maxValue)
                {
                    maxValue = a;
                }
                else if (comparator(*maxValue, a))
                {
                    maxValue = a;
                }
            });
            return maxValue;
        }

        auto max()
        {
            return max([](auto&& lhs, auto&& rhs) { return lhs < rhs; });
        }

        auto sum()
        {
            return reduce([](auto&& a, auto&& b) { return a + b; });
        }

        auto count()
        {
            std::size_t n = 0;
            while (enumerator_.next()) ++n;
            return n;
        }

        template <typename TPredicate>
        auto anyMatch(TPredicate predicate)
        {
            while (enumerator_.next())
            {
                if (predicate(enumerator_.current()))
                {
                    return true;
                }
            }
            return false;
        }

        template <typename TPredicate>
        auto allMatch(TPredicate predicate)
        {
            while (enumerator_.next())
            {
                if (!predicate(enumerator_.current()))
                {
                    return false;
                }
            }
            return true;
        }

        template <typename TPredicate>
        auto noneMatch(TPredicate predicate)
        {
            while (enumerator_.next())
            {
                if (!predicate(enumerator_.current()))
                {
                    return true;
                }
            }
            return false;
        }

    private:
        TEnumerable enumerator_;
    };
}

#include <cr/common/streams_from.h>
#include <cr/common/streams_empty.h>
#include <cr/common/streams_for.h>
#include <cr/common/streams_generate.h>

#endif
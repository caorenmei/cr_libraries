#ifndef CR_COMMON_STREAMS_FROM_H_
#define CR_COMMON_STREAMS_FROM_H_

#include <iterator>

namespace cr
{

    namespace streams
    {
        /** 范围迭代器. */
        template <typename TForwardIterator>
        class RangeEnumerator
        {
        public:
            typedef typename std::iterator_traits<TForwardIterator>::value_type ValueType;
            typedef typename std::iterator_traits<TForwardIterator>::reference ReferenceType;

            RangeEnumerator(TForwardIterator first, TForwardIterator last)
                : first_(first), 
                last_(last),
                count_(0)
            {}

            bool next()
            {
                if (count_++ > 0)
                {
                    ++first_;
                }
                return first_ != last_;
            }

            ReferenceType current()
            {
                return *first_;
            }

        private:
            TForwardIterator first_;
            TForwardIterator last_;
            std::size_t count_;
        };
    }

    template <typename TForwardIterator>
    auto from(TForwardIterator first, TForwardIterator last)
    {
        auto enumerator = streams::RangeEnumerator<TForwardIterator>{ first, last };
        return Stream<decltype(enumerator)>{ enumerator };
    }

    template <typename T>
    auto from(const std::initializer_list<T>& sequence)
    {
        return from(sequence.begin(), sequence.end());
    }

    template <typename TSequence>
    auto from(TSequence& sequence)
    {
        using std::begin;
        using std::end;
        return from(begin(sequence), end(sequence));
    }

    template <typename TSequence>
    auto from(const TSequence& sequence)
    {
        using std::begin;
        using std::end;
        return from(begin(sequence), end(sequence));
    }
}

#endif

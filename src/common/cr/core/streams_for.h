#ifndef CR_COMMON_STREAMS_FOR_H_
#define CR_COMMON_STREAMS_FOR_H_

#include <iterator>

#include <boost/optional.hpp>

namespace cr
{
    namespace streams
    {
        template <typename TEnumerator>
        class RangeForIterator
        {
        public:

            explicit RangeForIterator(boost::optional<TEnumerator> enumerator)
                : enumerator_(std::move(enumerator)),
                hasNext_(enumerator && enumerator_->next())
            {}

            typename TEnumerator::ReferenceType operator*()
            {
                return enumerator_->current();
            }

            RangeForIterator& operator++()
            {
                hasNext_ = enumerator_->next();
                return *this;
            }

        private:

            friend bool operator!=(const cr::streams::RangeForIterator<TEnumerator>& lhs, const cr::streams::RangeForIterator<TEnumerator>&/* rhs*/)
            {
                return lhs.hasNext_;
            }

            boost::optional<TEnumerator> enumerator_;
            bool hasNext_;
        };
    }

    template <typename TEnumerator>
    auto begin(Stream<TEnumerator>& stream)
    {
        auto enumerator = boost::optional<TEnumerator>{ std::move(stream.getEnumerator()) };
        return streams::RangeForIterator<TEnumerator>{ enumerator };
    }

    template <typename TEnumerator>
    auto end(Stream<TEnumerator>& stream)
    {
        return streams::RangeForIterator<TEnumerator>({});
    }
}

#endif

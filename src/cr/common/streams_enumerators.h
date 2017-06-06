#ifndef CR_COMMON_STREAMS_BASE_H_
#define CR_COMMON_STREAMS_BASE_H_

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

namespace cr
{
    namespace streams
    {
        
        /** 过滤器 */
        template <typename TEnumerable, typename TPredicate>
        class FilterEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            explicit FilterEnumerator(TEnumerable enumerator, TPredicate predicate)
                : enumerator_(std::move(enumerator)),
                predicate_(std::move(predicate))
            {}

            bool next()
            {
                while (enumerator_.next())
                {
                    if (predicate_(enumerator_.current()))
                    {
                        return true;
                    }
                }
                return false;
            }

            ReferenceType current()
            {
                return enumerator_.current();
            }

        private:
            TEnumerable enumerator_;
            TPredicate predicate_;
        };

        /** 深度拷贝 */
        template <typename TEnumerable>
        class DeepCloneEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            explicit DeepCloneEnumerator(TEnumerable enumerator)
                : enumerator_(std::move(enumerator))
            {}

            bool next()
            {
                if (enumerator_.next())
                {
                    current_.reset();
                    return true;
                }
                return false;
            }

            ReferenceType current()
            {
                if (!current_)
                {
                    current_ = enumerator_.current();
                }
                return *current_;
            }

        private:
            TEnumerable enumerator_;
            boost::optional<ValueType> current_;
        };

        /** 变换 */
        template <typename TEnumerable, typename TMapper>
        class MapperEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ValueType ReferenceType;

            explicit MapperEnumerator(TEnumerable enumerator, TMapper mapper)
                : enumerator_(std::move(enumerator)),
                mapper_(std::move(mapper))
            {}

            bool next()
            {
                return enumerator_.next();
            }

            ReferenceType current()
            {
                return mapper_(enumerator_.current());
            }

        private:
            TEnumerable enumerator_;
            TMapper mapper_;
        };

        /** 连接 */
        template <typename TEnumerable, typename UEnumerable>
        class ConcatEnumerator
        {
        public:

            typedef std::common_type_t<typename TEnumerable::ValueType, typename UEnumerable::ValueType> ValueType;
            typedef std::common_type_t<typename TEnumerable::ReferenceType, typename UEnumerable::ReferenceType> ReferenceType;

            ConcatEnumerator(TEnumerable firstEnumerator, UEnumerable secondEnumerator)
                : firstEnumerator_(std::move(firstEnumerator)),
                secondEnumerator_(std::move(secondEnumerator)),
                firstComsumed_(false)
            {}

            bool next()
            {
                if (!firstComsumed_)
                {
                    if (firstEnumerator_.next())
                    {
                        return true;
                    }
                    firstComsumed_ = true;
                }
                return secondEnumerator_.next();
            }

            ReferenceType current()
            {
                if (!firstComsumed_)
                {
                    return firstEnumerator_.current();
                }
                else
                {
                    return secondEnumerator_.current();
                }
            }

        private:
            TEnumerable firstEnumerator_;
            UEnumerable secondEnumerator_;
            bool firstComsumed_;
        };

        /** 去重 */
        template <typename TEnumerable, typename TPredicate>
        class DistinctEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            DistinctEnumerator(TEnumerable enumerator, TPredicate predicate)
                : enumerator_(std::move(enumerator)),
                predicate_(std::move(predicate))
            {}

            bool next()
            {
                if (current_)
                {
                    while (enumerator_.next())
                    {
                        boost::optional<ReferenceType> value = enumerator_.current();
                        if (!predicate_(*current_, *value))
                        {
                            current_ = *value;
                            return true;
                        }
                    }
                    current_.reset();
                    return false;
                }
                else if (enumerator_.next())
                {
                    current_ = enumerator_.current();
                    return true;
                }
                return false;
            }

            ReferenceType current()
            {
                return *current_;
            }

        private:
            TEnumerable enumerator_;
            TPredicate predicate_;
            boost::optional<ValueType> current_;
        };

        /** 排序 */
        template <typename TEnumerable, typename TComparator>
        class SortedEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            SortedEnumerator(TEnumerable enumerator, TComparator comparator)
                : enumerator_(std::move(enumerator)),
                comparator_(std::move(comparator)),
                current_(0)
            {}

            bool next()
            {
                if (!elements_)
                {
                    elements_ = std::make_shared<std::vector<ValueType> >();
                    while (enumerator_.next())
                    {
                        elements_->push_back(enumerator_.current());
                    }
                    std::sort(elements_->begin(), elements_->end(), comparator_);
                }
                else
                {
                    ++current_;
                }
                return current_ < elements_->size();
            }

            ReferenceType current()
            {
                return (*elements_)[current_];
            }

        private:
            TEnumerable enumerator_;
            TComparator comparator_;
            std::shared_ptr<std::vector<ValueType> > elements_;
            std::size_t current_;
        };

        /** 反转 */
        template <typename TEnumerable>
        class ReverseEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            explicit ReverseEnumerator(TEnumerable enumerator)
                : enumerator_(std::move(enumerator)),
                current_(0)
            {}

            bool next()
            {
                if (!elements_)
                {
                    elements_ = std::make_shared<std::vector<ValueType> >();
                    while (enumerator_.next())
                    {
                        elements_->push_back(enumerator_.current());
                    }
                    std::reverse(elements_->begin(), elements_->end());
                }
                else
                {
                    ++current_;
                }
                return current_ < elements_->size();
            }

            ReferenceType current()
            {
                return (*elements_)[current_];
            }

        private:
            TEnumerable enumerator_;
            std::shared_ptr<std::vector<ValueType> > elements_;
            std::size_t current_;
        };

        /** 修改数据 */
        template <typename TEnumerable, typename TAction>
        class PeekEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            PeekEnumerator(TEnumerable enumerator, TAction action)
                : enumerator_(std::move(enumerator)),
                action_(std::move(action))
            {}

            bool next()
            {
                if (enumerator_.next())
                {
                    action_(enumerator_.current());
                    return true;
                }
                return false;
            }

            ReferenceType current()
            {
                return enumerator_.current();
            }

        private:
            TEnumerable enumerator_;
            TAction action_;
        };

        /** 前面几项 */
        template <typename TEnumerable>
        class LimitEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            LimitEnumerator(TEnumerable enumerator, std::size_t limit)
                : enumerator_(std::move(enumerator)),
                limit_(limit),
                current_(0)
            {}

            bool next()
            {
                return enumerator_.next() && ++current_ <= limit_;
            }

            ReferenceType current()
            {
                return enumerator_.current();
            }

        private:
            TEnumerable enumerator_;
            std::size_t limit_;
            std::size_t current_;
        };

        /** 跳过几项 */
        template <typename TEnumerable>
        class SkipEnumerator
        {
        public:

            typedef typename TEnumerable::ValueType ValueType;
            typedef typename TEnumerable::ReferenceType ReferenceType;

            SkipEnumerator(TEnumerable enumerator, std::size_t skip)
                : enumerator_(std::move(enumerator)),
                skip_(skip),
                current_(0)
            {}

            bool next()
            {
                bool hasNext = true;
                while (current_ < skip_ && (hasNext = enumerator_.next()))
                {
                    ++current_;
                }
                if (!hasNext)
                {
                    return false;
                }
                return enumerator_.next();
            }

            ReferenceType current()
            {
                return enumerator_.current();
            }

        private:
            TEnumerable enumerator_;
            std::size_t skip_;
            std::size_t current_;
        };

    }
}

#endif

#ifndef CR_COMMON_OPTONAL_H_
#define CR_COMMON_OPTONAL_H_

#include <boost/optional.hpp>

namespace cr
{
    /**
     * 可选值
     */
    template <typename T>
    class Optional : public boost::optional<T>
    {
    public:
        using boost::optional<T>::optional;

        /**
         * 元素存在时执行操作
         */
        template <typename Action>
        void ifPresent(Action&& action)
        {
            if (this->is_initialized())
            {
                action(**this);
            }
        }
    };
}

#endif

#ifndef CR_COMMON_LOCK_GUARDS_H_
#define CR_COMMON_LOCK_GUARDS_H_

#include <algorithm>
#include <mutex>
#include <vector>

namespace cr
{
    /**
     * 对多个对象同时加锁和解锁
     */
    template <typename Mutex>
    class LockGuards
    {
    public:

        /**
         * 构造函数
         * @param mutexs 锁列表
         */
        explicit LockGuards(std::vector<std::unique_ptr<Mutex>>& mutexs)
            : mutexs_(mutexs)
        {
            std::for_each(mutexs_.begin(), mutexs_.end(), [](std::unique_ptr<Mutex>& mutex)
            {
                mutex->lock();
            });
        }

        /**
         * 析构函数，解锁
         */
        ~LockGuards()
        {
            std::for_each(mutexs_.rbegin(), mutexs_.rend(), [](std::unique_ptr<Mutex>& mutex)
            {
                mutex->unlock();
            });
        }

        LockGuards(const LockGuards&) = delete;
        LockGuards& operator=(const LockGuards&) = delete;

    private:

        std::vector<std::unique_ptr<Mutex>>& mutexs_;
    };
}

#endif

#ifndef CR_CONCURRENT_MULTI_MUTEX_H_
#define CR_CONCURRENT_MULTI_MUTEX_H_

#include <memory>
#include <mutex>
#include <vector>

namespace cr
{
    namespace concurrent
    {
        /**
         * 分段锁，减少锁的粒度
         */
        template <typename Mutex>
        class MultiMutex
        {
        public:

            /**
             * 构造函数
             * @param n 锁的个数
             */
            explicit MultiMutex(std::size_t n)
            {
                for (std::size_t i = 0; i != n; ++i)
                {
                    mutexs_.push_back(std::make_unique<Mutex>());
                }
            }

            /** 析构函数 */
            ~MultiMutex()
            {}

            MultiMutex(const MultiMutex&) = delete;
            MultiMutex& operator=(const MultiMutex&) = delete;

            /**
             * 加锁
             */
            void lock()
            {
                for (auto iter = mutexs_.begin(); iter != mutexs_.end(); ++iter)
                {
                    (*iter)->lock();
                }
            }

            /**
             * 解锁
             */
            void unlock()
            {
                for (auto iter = mutexs_.rbegin(); iter != mutexs_.rend(); ++iter)
                {
                    (*iter)->unlock();
                }
            }

            /**
             * 获取槽位上的锁
             * @param index 槽的索引
             * @return 锁
             */
            Mutex& slot(std::size_t index)
            {
                return *mutexs_[index];
            }

            /**
             * 槽位的数量
             */
            std::size_t size() const
            {
                return mutexs_.size();
            }

        private:

            std::vector<std::unique_ptr<Mutex>> mutexs_;
        };
    }
}

#endif

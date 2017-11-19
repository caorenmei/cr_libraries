#ifndef CR_COMMON_ZK_WATCH_MANAGER_H_
#define CR_COMMON_ZK_WATCH_MANAGER_H_

#include <map>
#include <memory>
#include <set>

#include "watcher.h"

namespace cr
{
    namespace zk
    {
        /** 监视器管理器 */
        class WatchManager
        {
        public:

            /** 构造函数 */
            WatchManager();

            /** 析构函数 */
            ~WatchManager();

            /**
             * 添加监视器
             * @param path 路径
             * @param watcher 监视器
             */
            void addWatcher(const std::string& path, std::shared_ptr<Watcher> watcher);

            /**
             * 移除监视器
             * @param watcher 监视器
             */
            void removeWatcher(const std::shared_ptr<Watcher>& watcher);

            /**
             * 触发监视
             * @param path 路径
             * @param type 事件
             * @return 触发的监视器
             */
            std::set<std::shared_ptr<Watcher>> triggerWatch(const std::string& path, EventType type);

            /**
             * 触发监视
             * @param path 路径
             * @param type 事件
             * @param supress 不需触发的监视器
             * @return 触发的监视器
             */
            std::set<std::shared_ptr<Watcher>> triggerWatch(const std::string& path, EventType type,
                const std::set<std::shared_ptr<Watcher>>& supress);

        private:

            // 路径 -> 监视器
            std::map<std::string, std::set<std::shared_ptr<Watcher>>> path2Watchs;
            // 监视器 -> 路径
            std::map<std::shared_ptr<Watcher>, std::set<std::string>> watch2Paths_;
        };
    }
}

#endif

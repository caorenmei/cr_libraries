#include "watch_manager.h"

#include <cassert>

namespace cr
{
    namespace zk
    {

        WatchManager::WatchManager()
        {}

        WatchManager::~WatchManager()
        {}

        void WatchManager::addWatcher(const std::string& path, std::shared_ptr<Watcher> watcher)
        {
            path2Watchs[path].insert(watcher);
            watch2Paths_[watcher].insert(path);
        }

        void WatchManager::removeWatcher(const std::shared_ptr<Watcher>& watcher)
        {
            auto watchIter = watch2Paths_.find(watcher);
            if (watchIter != watch2Paths_.end())
            {
                for (auto& path : watchIter->second)
                {
                    auto pathIter = path2Watchs.find(path);
                    assert(pathIter != path2Watchs.end());
                    pathIter->second.erase(watcher);
                    if (pathIter->second.empty())
                    {
                        path2Watchs.erase(pathIter);
                    }
                }
                watch2Paths_.erase(watchIter);
            }
        }

        std::set<std::shared_ptr<Watcher>> WatchManager::triggerWatch(const std::string& path, EventType type)
        {
            return triggerWatch(path, type, std::set<std::shared_ptr<Watcher>>());
        }

        std::set<std::shared_ptr<Watcher>> WatchManager::triggerWatch(const std::string& path, EventType type,
            const std::set<std::shared_ptr<Watcher>>& supress)
        {
            WatcherEvent event(type, ZkState::CONNECTED, path);
            std::set<std::shared_ptr<Watcher>> watchers;
            auto pathIter = path2Watchs.find(path);
            if (pathIter != path2Watchs.end())
            {
                watchers = std::move(pathIter->second);
                path2Watchs.erase(pathIter);
            }
            for (auto& watcher : watchers)
            {
                auto watchIter = watch2Paths_.find(watcher);
                assert(watchIter != watch2Paths_.end());
                watchIter->second.erase(path);
                if (watchIter->second.empty())
                {
                    watch2Paths_.erase(watchIter);
                }
            }
            for (auto& watcher : watchers)
            {
                if (supress.count(watcher) == 0)
                {
                    watcher->process(event);
                }
            }
            return watchers;
        }
    }
}
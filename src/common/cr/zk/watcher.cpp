#include "watcher.h"

namespace cr
{
    namespace zk
    {

        WatcherEvent::WatcherEvent(EventType event, ZkState state, std::string path)
            : event_(event),
            state_(state),
            path_(std::move(path))
        {}

        WatcherEvent::~WatcherEvent()
        {}

        EventType WatcherEvent::getEvent() const
        {
            return event_;
        }

        ZkState WatcherEvent::getState() const
        {
            return state_;
        }

        const std::string& WatcherEvent::getPath() const
        {
            return path_;
        }

        Watcher::Watcher(std::function<void(const WatcherEvent&)> cb)
            : callback_(std::move(cb))
        {}

        Watcher::~Watcher()
        {}

        void Watcher::process(const WatcherEvent& event)
        {
            callback_(event);
        }
    }
}
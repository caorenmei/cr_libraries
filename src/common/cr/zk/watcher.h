#ifndef CR_COMMON_ZK_WATCHER_H_
#define CR_COMMON_ZK_WATCHER_H_

#include <functional>
#include <string>

namespace cr
{
    namespace zk
    {

        /** 会话状态 */
        enum class ZkState
        {
            /** 未知状态*/
            NONE = 0,
            /** 客户端连接 */
            CONNECTED = 1,
            /** 客户端断开 */
            DISCONNECTED = 2,
            /** 客户端超时 */
            EXPIRED = 3,
        };

        /** 事件类型 */
        enum class EventType
        {
            /** 空事件 */
            NONE = 0,
            /** 创建节点 */
            NODE_CREATED = 1,
            /** 删除节点 */
            NODE_DELETE = 2,
            /** 节点数据改变 */
            NODE_DATA_CHANGED = 3,
            /** 子节点改变 */
            NODE_CHILDREN_CHANGED = 4,
        };

        /** 监听事件 */
        class WatcherEvent
        {
        public:

            /** 
             * 构造函数
             * @param event 事件
             * @param state 状态
             * @param path 路径
             */
            WatcherEvent(EventType event, ZkState state, std::string path);

            /** 析构函数 */
            ~WatcherEvent();

            /**
             * 获取事件
             * @return 事件
             */
            EventType getEvent() const;

            /**
             * 获取状态
             * @return 状态
             */
            ZkState getState() const;

            /**
             * 获取路径
             * @return 路径
             */
            const std::string& getPath() const;
            
        private:

            // 事件
            EventType event_;
            // 状态
            ZkState state_;
            // 路径
            std::string path_;
        };

        /** 监视器 */
        class Watcher
        {
        public:

            /**
             * 构造函数
             * @param 触发回调
             */
            explicit Watcher(std::function<void(const WatcherEvent&)> cb);

            /** 析构函数 */
            ~Watcher();

            /**
             * 触发监视器
             * @param event 事件
             */
            void process(const WatcherEvent& event);

        private:

            // 回调
            std::function<void(const WatcherEvent&)> callback_;
        };
    }
}
#endif

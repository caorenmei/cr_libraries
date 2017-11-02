#ifndef CR_COMMON_RAFT_RAFT_STATE_H_
#define CR_COMMON_RAFT_RAFT_STATE_H_

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/back/state_machine.hpp>

namespace cr
{
    namespace raft
    {
        namespace pb
        {
            /** 节点通信协议 */
            class RaftMsg;
        }

        /** raft算法 */
        class Raft;

        /** 伙伴节点 */
        class BuddyNode;

        /** 状态基类 */
        class BaseState
        {
        public:

            /** 析构函数 */
            virtual ~BaseState() = default;

            /**
             * 状态逻辑处理
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            virtual std::uint64_t update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages) = 0;
        };

        /** raft状态定义 */
        class RaftState_;
        using RaftState = boost::msm::back::state_machine<RaftState_>;

        /* 初始事件 */
        struct StartUpEvent {};
        /* 停止事件 */
        struct FinalEvent {};
        /* 选举超时事件 */
        struct ElectionTimeoutEvent {};
        /* 得到多数选票事件 */
        struct MajorityVotesEvent {};
        /* 收到跟随消息事件 */
        struct DiscoversEvent {};

        /** 跟随者状态 */
        class FollowerState;
        /** 候选者状态 */
        class CandidateState;
        /** 领导者状态 */
        class LeaderState;

        /** raft状态 */
        class RaftState_ : public boost::msm::front::state_machine_def<RaftState_, BaseState>
        {
        public:

            /* 初始状态 */
            using initial_state = boost::mpl::vector<FollowerState>;

            /* 状态转换表 */
            struct transition_table : boost::mpl::vector<
                boost::msm::front::Row<FollowerState, ElectionTimeoutEvent, CandidateState>,
                boost::msm::front::Row<CandidateState, MajorityVotesEvent, LeaderState>,
                boost::msm::front::Row<CandidateState, DiscoversEvent, FollowerState>,
                boost::msm::front::Row<LeaderState, DiscoversEvent, FollowerState>
            > {};

            /** 构造函数 */
            explicit RaftState_(Raft& raft);

            /** 析构函数 */
            ~RaftState_();

            RaftState_(const RaftState_&) = delete;
            RaftState_& operator=(const RaftState_&) = delete;

            /* 进入状态 */
            void on_entry(const StartUpEvent&, RaftState&);

            /* 离开状态机 */
            void on_exit(const StartUpEvent&, RaftState&);

            /**
             * 状态逻辑处理
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            virtual std::uint64_t update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

            /** 
             * 获取raft
             * @return raft
             */
            Raft& getRaft();

            /** 
             * 获取raft
             * @return raft
             */
            const Raft& getRaft() const;

            /**
             * 设置当前时间
             */
            void setNowTime(std::uint64_t nowTime);

            /**
             * 获取当前时间
             */
            std::uint64_t getNowTime() const;

            /** 
             * 是否是跟随者 
             * @return true为跟随者，false其它
             */
            bool isFollower() const;

            /** 
             * 是否是候选者
             * @return true为候选者，false其它
             */
            bool isCandidate() const;

            /** 
             * 是否是领导者
             * @return true为领导者，false其它
             */
            bool isLeader() const;

            /**
             * 获取消息队列
             * @return 消息队列
             */
            std::deque<std::shared_ptr<pb::RaftMsg>>& getMessages();

            /**
             * 获取消息队列
             * @return 消息队列
             */
            const std::deque<std::shared_ptr<pb::RaftMsg>>& getMessages() const;

        private:

            /* 随机选举超时时间 */
            std::uint64_t randElectionTimeout() const;

            // 友元
            friend class FollowerState;
            friend class CandidateState;
            friend class LeaderState;

            // raft 
            Raft& raft_;
            // 当前时间
            std::uint64_t nowTime_;
            // 消息队列
            std::deque<std::shared_ptr<pb::RaftMsg>> messages_;
        };

        /** 跟随状态 */
        class FollowerState : public boost::msm::front::state<BaseState, boost::msm::front::sm_ptr>
        {
        public:

            /** 构造函数 */
            FollowerState();

            /** 析构函数 */
            ~FollowerState();

            FollowerState(const FollowerState&) = delete;
            FollowerState& operator=(const FollowerState&) = delete;

            /* 进入状态 */
            template <typename Event, typename FSM>
            void on_entry(const Event&, FSM&)
            {}

            /* 退出状态 */
            template <typename Event, typename FSM>
            void on_exit(const Event&, FSM&)
            {}

            /* 初始化 */
            void on_entry(const StartUpEvent&, RaftState&);

            /* 成为跟随者 */
            void on_entry(const DiscoversEvent&, RaftState&);

            /* 设置状态机 */
            void set_sm_ptr(RaftState* sm);

            /**
            * 状态逻辑处理
            * @param messages 输出消息
            * @return 下一次需要update的时间
            */
            virtual std::uint64_t update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

        private:

            // 处理追加日志消息
            void handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执追加日志消息
            void sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 处理投票消息
            void handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执投票消息
            void sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 状态机
            RaftState* state_;
            // 下一个超时时间
            std::uint64_t electionTime_;
        };

        /** 候选者状态 */
        class CandidateState : public boost::msm::front::state<BaseState, boost::msm::front::sm_ptr>
        {
        public:

            /** 构造函数 */
            CandidateState();

            /** 析构函数 */
            ~CandidateState();

            CandidateState(const CandidateState&) = delete;
            CandidateState& operator=(const CandidateState&) = delete;

            /* 进入状态 */
            template <typename Event, typename FSM>
            void on_entry(const Event&, FSM&)
            {}

            /* 退出状态 */
            template <typename Event, typename FSM>
            void on_exit(const Event&, FSM&)
            {}

            /* 进入状态 */
            void on_entry(const ElectionTimeoutEvent&, RaftState&);

            /* 设置状态机 */
            void set_sm_ptr(RaftState* sm);

            /**
            * 状态逻辑处理
            * @param messages 输出消息
            * @return 下一次需要update的时间
            */
            virtual std::uint64_t update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

        private:

            // 处理追加日志消息
            bool handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执追加日志消息
            void sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 处理投票消息
            bool handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执投票消息
            void sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 处理投票回复消息
            bool handleRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 发送投票请求
            void sendRequestVoteReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 状态机
            RaftState* state_;
            // 选举时间
            std::uint64_t electionTime_;
            // 选票
            std::set<std::uint64_t> votes_;
        };

        /** 领导则状态 */
        class LeaderState : public boost::msm::front::state<BaseState, boost::msm::front::sm_ptr>
        {
        public:

            /** 构造函数 */
            LeaderState();

            /** 析构函数 */
            ~LeaderState();

            LeaderState(const LeaderState&) = delete;
            LeaderState& operator=(const LeaderState&) = delete;

            /* 进入状态 */
            template <typename Event, typename FSM>
            void on_entry(const Event&, FSM&)
            {}

            /* 退出状态 */
            template <typename Event, typename FSM>
            void on_exit(const Event&, FSM&)
            {}

            /* 得到多数选票 */
            void on_entry(const MajorityVotesEvent&, RaftState&);

            /* 设置状态机 */
            void set_sm_ptr(RaftState* sm);

            /**
            * 状态逻辑处理
            * @param messages 输出消息
            * @return 下一次需要update的时间
            */
            virtual std::uint64_t update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

        private:

            // 处理追加日志消息
            bool handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执追加日志消息
            void sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 处理投票消息
            bool handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 回执投票消息
            void sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 追加日志回复消息
            bool handleAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 发送心跳超时消息
            void sendAppendEntriesReq(BuddyNode& buddy, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 传输日志
            void transferAppendEntriesReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 广播心跳超时消息
            void broadcastAppendEntriesReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            // 状态机
            RaftState* state_;
            // 下一次心跳时间
            std::uint64_t heatbeatTime_;
            // 伙伴节点
            std::vector<BuddyNode> nodes_;
        };
    }
}

#endif

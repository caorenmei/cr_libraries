﻿#ifndef CR_COMMON_RAFT_RAFT_STATE_H_
#define CR_COMMON_RAFT_RAFT_STATE_H_

#include <cstdint>
#include <memory>
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

        /** 状态基类 */
        class BaseState
        {
        public:

            /** 析构函数 */
            virtual ~BaseState() = default;

            /**
             * 状态逻辑处理
             * @param nowTime 当前时间
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages) = 0;
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
                boost::msm::front::Row<CandidateState, ElectionTimeoutEvent, ElectionTimeoutEvent>,
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
             * @param nowTime 当前时间
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

            /** 
             * 获取raft
             * @return raft
             */
            Raft& getRaft();

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

        protected:

            // raft 
            Raft& raft_;
        };
    }
}

#endif

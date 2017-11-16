#ifndef CR_COMMON_RAFT_RAFT_H_
#define CR_COMMON_RAFT_RAFT_H_

#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <random>

#include <boost/optional.hpp>

#include "options.h"
#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        /** Raft 算法 */
        class Raft
        {
        public:

            /** 状态机执行结果 */
            enum ExecuteResult
            {
                /** 成功 */
                RESULT_COMMITTED = 0,
                /** 日志截断 */
                RESULT_LOG_TRUNC = 1,
                /** 失去领导者状态 */
                RESULT_LEADER_LOST = 2,
            };

            /**
             * 构造函数
             * @param options 参数
             */
            explicit Raft(const Options& options);

            /** 析构函数 */
            ~Raft();

            Raft(const Raft&) = delete;
            Raft& operator=(const Raft&) = delete;

            /**
             * 获取状态
             * @return 状态
             */
            const IRaftState& getState() const;

            /**
             * 获取raft参数
             * @return raft参数
             */
            const Options& getOptions() const;

            /**
             * 获取伙伴节点Id列表
             */
            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            /**
             * 开始raft算法
             * @param nowTime 当前时间
             */
            void start(std::uint64_t nowTime);

            /**
             * 获取当前选票
             * @return 当前选票
             */
            const boost::optional<std::uint64_t>& getVotedFor() const;

            /**
             * 获取当前领导者
             * @return领导者Id
             */
            const boost::optional<std::uint64_t>& getLeaderId() const;

            /**
             * 获取当前任期
             * @return 当前任期
             */
            std::uint64_t getCurrentTerm() const;

            /**
             * 获取提交的日志索引
             * @return 日志索引
             */
            std::uint64_t getCommitIndex() const;

            /**
             * 获取应用的日志索引
             * @return 应用的日志索引
             */
            std::uint64_t getLastApplied() const;

            /**
             * 状态逻辑处理
             * @param nowTime 当前时间
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            std::uint64_t update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages);

            /**
             * 接受到消息
             * @param message 其它节点的消息
             */
            void receive(std::shared_ptr<pb::RaftMsg> message);

            /**
             * 执行一条日志
             * @param value 日志数据
             * @return first 日志索引, second true成功，false失败
             */
            std::pair<std::uint64_t, bool> execute(const std::string& value);

            /**
             * 执行一条日志
             * @param value 日志数据
             * @param cb 提交回调
             * @return first 日志索引, second true成功，false失败
             */
            std::pair<std::uint64_t, bool> execute(const std::string& value, std::function<void(std::uint64_t, int)> cb);

        private:

            /* 获取随机数 */
            std::mt19937& getRandom();

            /* 设置选票投票Id */
            void setVotedFor(const boost::optional<std::uint64_t>& votedFor);

            /* 设置领导者Id */
            void setLeaderId(const boost::optional<std::uint64_t>& leaderId);

            /* 设置当前任期 */
            void setCurrentTerm(std::uint64_t currentTerm);

            /* 设置当前提交的日志索引 */
            void setCommitIndex(std::uint64_t commitIndex);

            // 应用日志
            void applyLog();

            // 丢失领导者状态
            void leaderLost();

            // 友元
            friend class RaftState_;
            friend class FollowerState;
            friend class CandidateState;
            friend class LeaderState;

            // 状态机
            RaftState state_;
            // 参数
            Options options_;
            // 伙伴节点Id列表
            std::vector<std::uint64_t> buddyNodeIds_;
            // 随机数
            std::mt19937 random_;
            // 当前投票
            boost::optional<std::uint64_t> votedFor_;
            // 当前领导者
            boost::optional<std::uint64_t> leaderId_;
            // 当前任期
            std::uint64_t currentTerm_;
            // 当前提交日志的日志索引
            std::uint64_t commitIndex_;
            // 最后应用到状态机的日志索引
            std::uint64_t lastApplied_;
            // 提交回调
            std::map<std::uint64_t, std::function<void(std::uint64_t, int)>> callbacks_;
        };
    }
}

#endif
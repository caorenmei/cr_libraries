#ifndef CR_COMMON_RAFT_RAFT_H_
#define CR_COMMON_RAFT_RAFT_H_

#include <cstdint>
#include <deque>
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
            RaftState& getStae();

            /**
             * 获取状态
             * @return 状态
             */
            const RaftState& getState() const;

            /**
             * 获取raft参数
             * @return raft参数
             */
            const Options& getOptions() const;

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
             * 运行日志
             * @param logEntryNum 运行的日志条目
             * @return true还需继续运行，false其他
             */
            bool execute(std::size_t logEntryNum = 10);

        private:

            /* 设置选票投票Id */
            void setVotedFor(const boost::optional<std::uint64_t>& votedFor);

            /* 设置领导者Id */
            void setLeaderId(const boost::optional<std::uint64_t>& leaderId);

            /* 设置当前任期 */
            void setCurrentTerm(std::uint64_t currentTerm);

            /* 设置当前提交的日志索引 */
            void setCommitIndex(std::uint64_t commitIndex);

            // 状态机
            RaftState state_;
            // 参数
            Options options_;
            // 随机数
            std::default_random_engine random_;
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
        };
    }
}

#endif
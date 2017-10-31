﻿#ifndef CR_RAFT_RAFT_STATE_H_
#define CR_RAFT_RAFT_STATE_H_

#include <cstdint>
#include <memory>
#include <vector>

namespace cr
{
    namespace raft
    {
        // 消息
        namespace pb
        {
            class RaftMsg;
        }

        class Raft;

        class RaftState
        {
        public:

            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            explicit RaftState(Raft& raft);

            virtual ~RaftState();

            virtual int getState() const = 0;

            virtual void onEnter(std::shared_ptr<RaftState> prevState) = 0;

            virtual void onLeave() = 0;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) = 0;

        protected:

            Raft& raft;
        };
    }
}

#endif

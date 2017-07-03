#ifndef CR_RAFT_RAFT_STATE_H_
#define CR_RAFT_RAFT_STATE_H_

#include <cstdint>
#include <memory>
#include <vector>

namespace cr
{
    namespace raft
    {

        namespace pb
        {
            class RaftMsg;
        }

        class RaftEngine;

        class RaftState
        {
        public:

            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            explicit RaftState(RaftEngine& engine);

            virtual ~RaftState();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) = 0;

            virtual void onLeave() = 0;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) = 0;

        protected:

            RaftEngine& engine;
        };
    }
}

#endif

#include <cr/raft/follower.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {
        Follower::Follower(RaftEngine& engine)
            : RaftState(engine),
            lastHeartbeatTime_(0)
        {}

        Follower::~Follower()
        {}

        void Follower::onEnter(std::shared_ptr<RaftState> prevState)
        {
            lastHeartbeatTime_ = getEngine().getNowTime();
        }

        void Follower::onLeave()
        {

        }

        std::int64_t Follower::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            // 选举超时
            if (lastHeartbeatTime_ + getEngine().getElectionTimeout() <= nowTime)
            {
                getEngine().setNextState(RaftEngine::CANDIDATE);
                return nowTime;
            }
            
            // 选举超时时间之前需要update
            return std::max<std::int64_t>(lastHeartbeatTime_ + getEngine().getElectionTimeout() / 2, nowTime);
        }
    }
}
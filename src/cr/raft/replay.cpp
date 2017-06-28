#include <cr/raft/replay.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        Replay::Replay(RaftEngine& engine)
            : RaftState(engine)
        {}

        Replay::~Replay()
        {}

        void Replay::onEnter(std::shared_ptr<RaftState> prevState)
        {
            CR_ASSERT(prevState == nullptr);
        }

        void Replay::onLeave()
        {

        }

        std::int64_t Replay::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            const auto& logStorage = getEngine().getLogStorage();
            const auto& stateMachine = getEngine().getStateMachine();
            std::uint64_t lastApplied = getEngine().getLastApplied();
            std::uint64_t lastLogIndex = getEngine().getCommitLogIndex();
            if (lastApplied < lastLogIndex)
            {
                lastApplied = lastApplied + 1;
                // 应用日志
                LogEntry logEntry;
                logStorage->get(lastApplied, logEntry);
                stateMachine->execute(lastApplied, logEntry.getValue(), boost::any());
                // 修该应用状态
                getEngine().setLastApplied(lastApplied);
            }
            if (lastApplied == lastLogIndex)
            {
                getEngine().setNextState(RaftEngine::FOLLOWER);
            }
            return nowTime;
        }
    }
}
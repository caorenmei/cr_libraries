#ifndef CR_RAFT_RAFT_ENGINE_H_
#define CR_RAFT_RAFT_ENGINE_H_

#include <cstdint>
#include <deque>
#include <memory>

#include <boost/optional.hpp>

#include <cr/raft/storage.h>
#include <cr/raft/raft_engine_builder.h>

namespace cr
{
    namespace raft
    {

        namespace pb
        {
            class RaftMsg;
        }

        class RaftState;

        template <typename T>
        class DebugVisitor;

        class RaftEngine
        {
        public:

            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            using Builder = RaftEngineBuilder;

            enum State
            {
                FOLLOWER,
                CANDIDATE,
                LEADER,
            };

            explicit RaftEngine(const Builder& builder);

            ~RaftEngine();

            RaftEngine(const RaftEngine&) = delete;
            RaftEngine& operator=(const RaftEngine&) = delete;

            std::uint32_t getNodeId() const;

            const std::vector<std::uint32_t>& getBuddyNodeIds() const;

            bool isBuddyNodeId(std::uint32_t nodeId) const;

            const std::shared_ptr<Storage>& getStorage() const;

            std::uint64_t getNowTime() const;

            void initialize(std::uint64_t nowTime);

            std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            std::deque<RaftMsgPtr>& getMessageQueue();

            void execute(std::string value);

            std::uint32_t getHeatbeatTimeout() const;

            std::uint32_t getMinElectionTimeout() const;

            std::uint64_t getCommitIndex() const;

            std::uint64_t getLastApplied() const;

            State getCurrentState() const;

            std::uint32_t getCurrentTerm() const;

            boost::optional<std::uint32_t> getVotedFor() const;

            const boost::optional<std::uint32_t>& getLeaderId() const;

        private:

            template <typename T>
            friend class DebugVisitor;
         
            friend class Follower;
            friend class Candidate;
            friend class Leader;

            std::uint32_t getLogWindowSize() const;

            std::uint32_t getMaxPacketLength() const;

            void setNextState(State nextState);

            void onTransitionState();

            void setCurrentTerm(std::uint32_t currentTerm);

            void setVotedFor(boost::optional<std::uint32_t> voteFor);

            void setCommitIndex(std::uint64_t commitIndex);

            void setLeaderId(boost::optional<std::uint32_t> leaderId);

            std::uint32_t randomElectionTimeout() const;


            std::uint32_t nodeId_;
            std::vector<std::uint32_t> buddyNodeIds_;
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executeCallback_;
            std::function<std::uint32_t()> random_;
            std::uint32_t logWindowSize_;
            std::uint32_t maxPacketLength_;
            std::deque<RaftMsgPtr> messages_;

            std::uint32_t currentTerm_;
            boost::optional<std::uint32_t> votedFor_;

            std::uint64_t commitIndex_;
            std::uint64_t lastApplied_;
            boost::optional<std::uint32_t> leaderId_;
            std::pair<std::uint32_t, std::uint32_t> electionTimeout_;
            std::uint32_t heatbeatTimeout_;

            std::uint64_t nowTime_;
            std::shared_ptr<RaftState> currentState_;
            State currentEnumState_;
            State nextEnumState_;
        };
    }
}

#endif
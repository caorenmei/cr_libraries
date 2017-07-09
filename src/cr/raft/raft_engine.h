#ifndef CR_RAFT_RAFT_ENGINE_H_
#define CR_RAFT_RAFT_ENGINE_H_

#include <cstdint>
#include <deque>
#include <memory>
#include <random>

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

            enum State : int
            {
                FOLLOWER,
                CANDIDATE,
                LEADER,
            };

            explicit RaftEngine(const Builder& builder);

            ~RaftEngine();

            RaftEngine(const RaftEngine&) = delete;
            RaftEngine& operator=(const RaftEngine&) = delete;

            std::uint64_t getNodeId() const;

            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            bool isBuddyNodeId(std::uint64_t nodeId) const;

            const std::shared_ptr<Storage>& getStorage() const;

            std::uint64_t getNowTime() const;

            void initialize(std::uint64_t nowTime);

            std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            std::deque<RaftMsgPtr>& getMessageQueue();

            bool execute(const std::vector<std::string>& value);

            std::uint64_t getHeatbeatTimeout() const;

            std::uint64_t getMinElectionTimeout() const;

            std::uint64_t getCommitIndex() const;

            std::uint64_t getLastApplied() const;

            State getCurrentState() const;

            std::uint64_t getCurrentTerm() const;

            boost::optional<std::uint64_t> getVotedFor() const;

            const boost::optional<std::uint64_t>& getLeaderId() const;

        private:

            template <typename T>
            friend class DebugVisitor;
         
            friend class Follower;
            friend class Candidate;
            friend class Leader;

            std::uint64_t getMaxEntriesNum() const;

            std::uint64_t getMaxPacketLength() const;

            void setNextState(State nextState);

            void onTransitionState();

            void setCurrentTerm(std::uint64_t currentTerm);

            void setVotedFor(boost::optional<std::uint64_t> voteFor);

            void setCommitIndex(std::uint64_t commitIndex);

            void setLeaderId(boost::optional<std::uint64_t> leaderId);

            std::uint64_t randomElectionTimeout();

            std::default_random_engine random_;
            std::uint64_t minElectionTimeout_;
            std::uint64_t maxElectionTimeout_;
            std::uint64_t heatbeatTimeout_;
            std::uint64_t maxEntriesNum_;
            std::uint64_t maxPacketLength_;
            
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executable_;

            std::uint64_t nowTime_;
            std::deque<RaftMsgPtr> messages_;
            std::shared_ptr<RaftState> currentState_;
            State nextState_;

            std::uint64_t nodeId_;
            std::vector<std::uint64_t> buddyNodeIds_;
            boost::optional<std::uint64_t> votedFor_;
            boost::optional<std::uint64_t> leaderId_;

            std::uint64_t currentTerm_;
            std::uint64_t commitIndex_;
            std::uint64_t lastApplied_;
        };
    }
}

#endif
#ifndef CR_RAFT_RAFT_ENGINE_BUILDER_H_
#define CR_RAFT_RAFT_ENGINE_BUILDER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include <boost/optional.hpp>

#include <cr/raft/storage.h>

namespace cr
{
    namespace raft
    {
        /** Raft算法引擎 */
        class RaftEngine;

        /** Raft引擎构造器 */
        class RaftEngineBuilder
        {
        public:

            RaftEngineBuilder();

            ~RaftEngineBuilder();

            RaftEngineBuilder& setNodeId(std::uint64_t nodeId);

            std::uint64_t getNodeId() const;

            RaftEngineBuilder& setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds);

            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            RaftEngineBuilder& setStorage(std::shared_ptr<Storage> storage);

            const std::shared_ptr<Storage>& getStorage() const;

            RaftEngineBuilder& setEexcuteCallback(std::function<void(std::uint64_t, const std::string&)> cb);

            const std::function<void(std::uint64_t, const std::string&)>& getEexcuteCallback() const;

            RaftEngineBuilder& setRandomSeed(std::size_t seed);

            std::size_t getRandomSeed() const;

            RaftEngineBuilder& setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout);

            std::uint64_t getMinElectionTimeout() const;

            std::uint64_t getMaxElectionTimeout() const;

            RaftEngineBuilder& setHeartbeatTimeout(std::uint64_t heartbeatTimeout);

            std::uint64_t getHeatbeatTimeout() const;

            RaftEngineBuilder& setMaxEntriesNum(std::uint64_t maxEntriesNum);

            std::uint64_t getMaxEntriesNum() const;

            RaftEngineBuilder& setMaxPacketSize(std::uint64_t maxPacketSize);

            std::uint64_t getMaxPacketLength() const;

            std::shared_ptr<RaftEngine> build();

        private:

            std::uint64_t nodeId_;
            std::vector<std::uint64_t> buddyNodeIds_;
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executable_;
            std::size_t randomSeed_;
            std::uint64_t minElectionTimeout_;
            std::uint64_t maxElectionTimeout_;
            std::uint64_t heartbeatTimeout_;
            std::uint64_t maxEntriesNum_;
            std::uint64_t maxPacketLength_;
        };
    }
}

#endif



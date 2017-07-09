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

            RaftEngineBuilder& setElectionTimeout(const std::pair<std::uint64_t, std::uint64_t>& electionTimeout);

            const std::pair<std::uint64_t, std::uint64_t>& getElectionTimeout() const;

            RaftEngineBuilder& setHeartbeatTimeout(std::uint64_t heartbeatTimeout);

            std::uint64_t getHeatbeatTimeout() const;

            RaftEngineBuilder& setRandom(std::function<std::uint64_t()> random);

            const std::function<std::uint64_t()>& getRandom() const;

            RaftEngineBuilder& setLogWindowSize(std::uint64_t logWindowSize);

            std::uint64_t getLogWindowSize() const;

            RaftEngineBuilder& setMaxPacketSize(std::uint64_t maxPacketSize);

            std::uint64_t getMaxPacketLength() const;

            std::shared_ptr<RaftEngine> build();

        private:

            std::uint64_t nodeId_;
            std::vector<std::uint64_t> buddyNodeIds_;
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executable_;
            std::pair<std::uint64_t, std::uint64_t> electionTimeout_;
            std::uint64_t heartbeatTimeout_;
            std::function<std::uint64_t()> random_;
            std::uint64_t logWindowSize_;
            std::uint64_t maxPacketLength_;
        };
    }
}

#endif



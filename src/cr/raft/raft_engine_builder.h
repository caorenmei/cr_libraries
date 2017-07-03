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

            RaftEngineBuilder& setNodeId(std::uint32_t nodeId);

            std::uint32_t getNodeId() const;

            RaftEngineBuilder& setBuddyNodeIds(std::vector<std::uint32_t> otherNodeIds);

            const std::vector<std::uint32_t>& getBuddyNodeIds() const;

            RaftEngineBuilder& setStorage(std::shared_ptr<Storage> storage);

            const std::shared_ptr<Storage>& getStorage() const;

            RaftEngineBuilder& setEexcuteCallback(std::function<void(std::uint64_t, const std::string&)> cb);

            const std::function<void(std::uint64_t, const std::string&)>& getEexcuteCallback() const;

            RaftEngineBuilder& setElectionTimeout(const std::pair<std::uint32_t, std::uint32_t>& electionTimeout);

            const std::pair<std::uint32_t, std::uint32_t>& getElectionTimeout() const;

            RaftEngineBuilder& setRandom(std::function<std::uint32_t()> random);

            const std::function<std::uint32_t()>& getRandom() const;

            RaftEngineBuilder& setLogWindowSize(std::uint32_t logWindowSize);

            std::uint32_t getLogWindowSize() const;

            RaftEngineBuilder& setMaxPacketSize(std::uint32_t maxPacketSize);

            std::uint32_t getMaxPacketLength() const;

            std::shared_ptr<RaftEngine> build();

        private:

            std::uint32_t nodeId_;
            std::vector<std::uint32_t> buddyNodeIds_;
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executeCallback_;
            std::pair<std::uint32_t, std::uint32_t> electionTimeout_;
            std::function<std::uint32_t()> random_;
            std::uint32_t logWindowSize_;
            std::uint32_t maxPacketLength_;
        };
    }
}

#endif



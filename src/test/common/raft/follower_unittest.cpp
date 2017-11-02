#include <boost/test/unit_test.hpp>

#include <memory>
#include <vector>

#include <cr/raft/mem_storage.h>
#include <cr/raft/raft.h>
#include <cr/raft/raft_msg.pb.h>

#include "raft_utils.h"

BOOST_AUTO_TEST_SUITE(RaftFollower)

struct StatMachine
{
public:

    void execute(std::uint64_t logIndex, const std::string& value)
    {
        getEntries.push_back(value);
    }

    std::vector<std::string> getEntries;
};

struct FollowerFixture
{
    FollowerFixture()
    {
        cr::raft::Options options;
        options.setNodeId(0)
            .setBuddyNodeIds({ 1,2,3,4 })
            .setElectionTimeout(100, 100)
            .setHeartbeatTimeout(30);
        auto storage = std::make_shared<cr::raft::MemStorage>();
        options.setStorage(storage);
        options.setEexcutable([this](uint64_t, std::string value)
        {
            entries.push_back(std::move(value));
        });
        raft = std::make_unique<cr::raft::Raft>(options);
        raft->start(0);
    }

    ~FollowerFixture()
    {}

    std::vector<std::string> entries;
    std::unique_ptr<cr::raft::Raft> raft;
};

BOOST_FIXTURE_TEST_CASE(electionTimeout, FollowerFixture)
{
    std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages;
    auto& state = raft->getState();

    state.update(50, messages);
    BOOST_CHECK(state.isFollower());

    state.update(99, messages);
    BOOST_CHECK(state.isFollower());

    state.update(100, messages);
    BOOST_CHECK(!state.isFollower());
}

BOOST_FIXTURE_TEST_CASE(appendEntriesReq, FollowerFixture)
{
    std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages;
    auto& state = raft->getState();

    // 更新任期消息
    auto message = makeAppendEntriesReq(1, 0, 2, 0, 0, 1);
    appendLogEntry(*message, makeEntry(1, 1, "1"));
    appendLogEntry(*message, makeEntry(2, 2, "2"));
    state.getMessages().push_back(message);

}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/mem_log_storage.h>
#include <cr/raft/raft_engine.h>

BOOST_AUTO_TEST_SUITE(RaftEngine)

BOOST_AUTO_TEST_CASE(build)
{
    cr::raft::RaftEngine::Builder builder;
    builder.setNodeId(1)
        .setInstanceId(2)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(std::make_shared<cr::raft::MemLogStorage>());
   
    BOOST_CHECK_NO_THROW(builder.build());
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 1,3,4 }) .build(), cr::raft::RaftEngine::BuildException);
    BOOST_CHECK_THROW(builder.setOtherNodeIds({ 2,2,4 }).build(), cr::raft::RaftEngine::BuildException);
    BOOST_CHECK_THROW(builder.setLogStorage(nullptr).build(), cr::raft::RaftEngine::BuildException);

    auto raftEngine = builder.setNodeId(1)
        .setInstanceId(2)
        .setOtherNodeIds({ 2,3,4 })
        .setLogStorage(std::make_shared<cr::raft::MemLogStorage>())
        .build();

    BOOST_CHECK_EQUAL(raftEngine->getNodeId(), 1);
    BOOST_CHECK_EQUAL(raftEngine->getInstanceId(), 2);
    BOOST_CHECK(cr::from(raftEngine->getOtherNodeIds()).sorted().equals(cr::from({ 2,3,4 })));
}

BOOST_AUTO_TEST_SUITE_END()

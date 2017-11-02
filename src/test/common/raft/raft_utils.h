#ifndef RAFT_UTILS_H_
#define RAFT_UTILS_H_

#include <memory>
#include <vector>

#include <cr/raft/raft_msg.pb.h>

// 新建追加日志请求
std::shared_ptr<cr::raft::pb::RaftMsg> makeAppendEntriesReq(
    std::uint64_t fromNodeId,
    std::uint64_t destNodeId,
    std::uint64_t leaderTerm,
    std::uint64_t prevLogIndex,
    std::uint64_t prevLogTerm,
    std::uint64_t leaderCommit);

// 新建日志条目
cr::raft::pb::Entry makeEntry(std::uint64_t index, std::uint64_t term, std::string value);

// 追加日志
void appendLogEntry(cr::raft::pb::RaftMsg& message, const cr::raft::pb::Entry& entry);

#endif

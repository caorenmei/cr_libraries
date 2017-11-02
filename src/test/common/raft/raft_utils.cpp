#include "raft_utils.h"

std::shared_ptr<cr::raft::pb::RaftMsg> makeAppendEntriesReq(
    std::uint64_t fromNodeId,
    std::uint64_t destNodeId,
    std::uint64_t leaderTerm,
    std::uint64_t prevLogIndex,
    std::uint64_t prevLogTerm,
    std::uint64_t leaderCommit)
{
    auto message = std::make_shared<cr::raft::pb::RaftMsg>();
    message->set_from_node_id(fromNodeId);
    message->set_dest_node_id(destNodeId);
    auto request = message->mutable_append_entries_req();
    request->set_leader_term(leaderTerm);
    request->set_prev_log_index(prevLogIndex);
    request->set_prev_log_term(prevLogTerm);
    request->set_leader_commit(leaderCommit);
    return message;
}

cr::raft::pb::Entry makeEntry(std::uint64_t index, std::uint64_t term, std::string value)
{
    cr::raft::pb::Entry entry;
    entry.set_index(index);
    entry.set_term(term);
    entry.set_value(std::move(value));
    return entry;
}

void appendLogEntry(cr::raft::pb::RaftMsg& message, const cr::raft::pb::Entry& entry)
{
    *message.mutable_append_entries_req()->add_entries() = entry;
}
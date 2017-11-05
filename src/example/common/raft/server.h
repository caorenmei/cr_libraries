#ifndef SERVER_H_
#define SERVER_H_

#include <cr/raft/mem_storage.h>
#include <cr/raft/raft.h>

// raft 服务器
class Server
{
public:

    // 构造函数
    Server(std::uint64_t nodeId, std::vector<std::uint64_t> buddyNodeIds, std::size_t randomSeed);

    // 析构函数
    ~Server();

    // 获取节点Id
    std::uint64_t getNodeId() const;

    // 当前时间
    void start(std::uint64_t nowTime);

    // 崩溃
    void crash(std::uint64_t nowTime, std::uint64_t durationTime);

    //接受消息
    void receive(std::shared_ptr<cr::raft::pb::RaftMsg> message);

    // 执行逻辑更新
    void update(std::uint64_t nowTime, std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>>& messages);

    // 服务是否有效
    bool isValid() const;

    // 是否是领导则
    bool isLeader() const;

    // 获取当前任期
    std::uint64_t getCurrentTerm() const;

    // 获取值
    const std::vector<cr::raft::pb::Entry>& getEntries() const;

    // 获取提交日志索引
    std::uint64_t getCommitIndex() const;

    // 获取校验过的日志索引
    std::uint64_t getCheckIndex() const;

    // 设置校验过的日志索引
    void setCheckIndex(std::uint64_t checkIndex);

    // 计算出来的值
    std::uint64_t getValue() const;

private:

    // 随机数
    std::default_random_engine random_;
    // 节点Id
    std::uint64_t nodeId_;
    // 伙伴节点Id
    std::vector<uint64_t> buddyNodeIds_;
    // 存储
    std::shared_ptr<cr::raft::MemStorage> storage_;
    // crash时间
    std::uint64_t crashEndTime_;
    // raft 算法
    std::unique_ptr<cr::raft::Raft> raft_;
    // 是否是领导者
    bool leader_;
    // 校验索引
    std::uint64_t checkIndex_;
    // 值序列
    std::uint64_t value_;
};

#endif

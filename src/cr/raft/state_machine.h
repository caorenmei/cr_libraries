﻿#ifndef CR_RAFT_STATE_MACHINE_H_
#define CR_RAFT_STATE_MACHINE_H_

#include <boost/any.hpp>

#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        /** Raft 状态机 */
        class StateMachine
        {
        public:

            /** 构造函数 */
            inline StateMachine() {}

            /** 析构函数 */
            virtual ~StateMachine() {}

            /**
             * 执行状态机指令
             * @param instanceId 实例Id
             * @param logIndex 索引值
             * @param value 日志数据
             * @param ctx 上下文
             */
            virtual void execute(std::uint32_t instanceId, std::uint64_t logIndex, const std::string& value, boost::any ctx) = 0;
            
        };
    }
}

#endif

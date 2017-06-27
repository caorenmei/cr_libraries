#ifndef CR_RAFT_ERROR_CODE_H_
#define CR_RAFT_ERROR_CODE_H_

namespace cr
{
    namespace raft
    {
        namespace error
        {
            /** raft 涉及的错误码 */
            enum Errors : int
            {
                /** 操作成功 */
                SUCCESS,
                /** 不支持的操作 */
                NOT_SUPPORT,
                /** IO异常 */
                IO_ERROR,
                /** 序号错误 */
                LOG_INDEX_ERROR,
                /** 日志数据错误 */
                LOG_DATA_ERROR,
                /** 没有该日志  */
                NO_LOG_INDEX,
                /** 实例没有日志  */
                NO_LOG_DATA,
                /** 没有快照 */
                NO_CHECKPOINT,
                /** 日志已生成快照 */
                IN_CHECKPOINT,
            };
        }
    }
}

#endif

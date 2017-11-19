#ifndef CR_COMMON_ZK_CREATE_MODE_H_
#define CR_COMMON_ZK_CREATE_MODE_H_

namespace cr
{
    namespace zk
    {
        /** 节点创建模式 */
        class CreateMode
        {
        public:

            /** 持久节点 */
            static const CreateMode PERSISTENT;

            /** 持久顺序节点 */
            static const CreateMode PERSISTENT_SEQUENTIAL;

            /** 临时节点 */
            static const CreateMode EPHEMERAL;

            /** 临时顺序节点 */
            static const CreateMode EPHEMERAL_SEQUENTIAL;

            /**
             * 构造函数
             * @param flag 整数表示
             */
            explicit CreateMode(int flag);

            /**
             * 构造函数
             * @param ephemeral 临时节点
             * @param sequential 序列节点
             */
            CreateMode(bool ephemeral, bool sequential);

            /** 析构函数 */
            ~CreateMode();

            /**
             * 是否是临时节点
             * @return true是临时节点，false其他
             */
            bool isEphemeral() const;

            /**
             * 是否是序列
             * @return true是，false其他
             */
            bool isSequential() const;

            /**
             * 整数表示
             * @reutrn 0 持久节点，1持久顺序节点，2临时节点，3临时顺序节点
             */
            int toFlag() const;

        private:

            // 临时节点 
            bool ephemeral_;
            // 序列节点
            bool sequential_;
            // 整数表示
            int flag_;
        };

    }
}

#endif

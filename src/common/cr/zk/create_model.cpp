#include "create_model.h"

namespace cr
{
    namespace zk
    {
        const CreateMode CreateMode::PERSISTENT(false, false);

        const CreateMode CreateMode::PERSISTENT_SEQUENTIAL(false, true);

        const CreateMode CreateMode::EPHEMERAL(true, false);

        const CreateMode CreateMode::EPHEMERAL_SEQUENTIAL(true, true);

        static constexpr int EPHEMERAL_VALUE = 1;
        static constexpr int SEQUENTIAL_VALUE = 2;

        CreateMode::CreateMode(int flag)
            : ephemeral_((flag & EPHEMERAL_VALUE) != 0),
            sequential_((flag & SEQUENTIAL_VALUE) != 0),
            flag_((flag & EPHEMERAL_VALUE) | (flag & SEQUENTIAL_VALUE))
        {}

        CreateMode::CreateMode(bool ephemeral, bool sequential)
            : ephemeral_(ephemeral),
            sequential_(sequential),
            flag_((ephemeral ? EPHEMERAL_VALUE : 0) | (sequential ? SEQUENTIAL_VALUE : 0))
        {}

        CreateMode::~CreateMode()
        {}

        bool CreateMode::isEphemeral() const
        {
            return ephemeral_;
        }

        bool CreateMode::isSequential() const
        {
            return sequential_;
        }

        int CreateMode::toFlag() const
        {
            return flag_;
        }
    }
}
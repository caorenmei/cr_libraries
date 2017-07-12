#include <cr/raft/file_storage.h>

namespace cr
{
    namespace raft
    {

        class FileStorage::Impl
        {};

        FileStorage::FileStorage(const std::string& path)
        {}

        FileStorage::~FileStorage()
        {}

        std::shared_ptr<Storage> FileStorage::newInstance(std::uint64_t instanceId)
        {
            return nullptr;
        }
    }
}
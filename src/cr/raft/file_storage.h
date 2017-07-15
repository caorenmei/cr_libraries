#ifndef CR_COMMON_FILE_STORAGE_H_
#define CR_COMMON_FILE_STORAGE_H_

#include <memory>

#include <cr/raft/storage.h>

namespace cr
{
    namespace raft
    {
        // 文件存储
        class FileStorage
        {
        public:

            explicit FileStorage(const std::string& path, bool sync = true);

            ~FileStorage();

            FileStorage(const FileStorage&) = delete;
            FileStorage& operator=(const FileStorage&) = delete;

            // 获取存储接口
            std::shared_ptr<Storage> getStorage(std::uint64_t instanceId);

        private:

            class Impl;
            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif

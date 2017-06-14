#include <cr/framework/common/path_directory.h>

#include <cctype>
#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#include <boost/functional/hash.hpp>

#include <cr/common/assert.h>

namespace cr
{
    namespace framework
    {
        // 实际实现
        class PathDirectory::Impl
        {
        public:
            // 节点数据
            std::unordered_map<std::string, std::string> datas;
            // 节点版本号
            std::unordered_map<std::string, std::set<std::uint32_t>> versions;
            // 第一层节点
            std::set<std::string> roots;
            // 子节点
            std::unordered_map<std::string, std::set<std::string>> children;
        };

        PathDirectoryException::PathDirectoryException(int category, std::string message)
            : category_(category),
            Exception(std::move(message))
        {}

        int PathDirectoryException::getCategoryCode() const
        {
            return category_;
        }

        PathDirectory::PathDirectory()
            : impl_(std::make_unique<Impl>())
        {}

        PathDirectory::PathDirectory(const PathDirectory& other)
            : impl_(std::make_unique<Impl>(*other.impl_))
        {}

        PathDirectory::PathDirectory(PathDirectory&& other)
            : impl_(std::move(other.impl_))
        {
            other.impl_ = std::make_unique<Impl>();
        }

        PathDirectory::~PathDirectory()
        {}

        PathDirectory& PathDirectory::operator=(const PathDirectory& other)
        {
            PathDirectory (other).swap(*this);
            return *this;
        }

        PathDirectory& PathDirectory::operator=(PathDirectory&& other)
        {
            PathDirectory(std::move(other)).swap(*this);
            return *this;
        }

        void PathDirectory::swap(PathDirectory& other)
        {
            std::swap(impl_, other.impl_);
        }

        bool PathDirectory::checkValidPath(const std::string& path)
        {
            enum CheckState { start, delimiter, character };
            CheckState state = start;
            for (std::size_t i = 0; i != path.size(); ++i)
            {
                switch (state)
                {
                case start:
                    if (path[i] != '/')
                    {
                        return false;
                    }
                    state = delimiter;
                    break;
                case delimiter:
                    if (!std::isalnum(path[i]))
                    {
                        return false;
                    }
                    state = character;
                    break;
                case character:
                    if (path[i] == '/')
                    {
                        state = delimiter;
                    }
                    else if (!std::isalnum(path[i]))
                    {
                        return false;
                    }
                    break;
                default:
                    CR_ASSERT(!"failed code path!!!");
                }
            }
            return state == character;
        }

        std::size_t PathDirectory::getRootChildrenCount() const
        {
            return impl_->roots.size();
        }
    }
}
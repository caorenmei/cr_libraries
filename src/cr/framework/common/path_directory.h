#ifndef CR_FRAMEWORK_COMMON_PATH_DIRECTORY_H_
#define CR_FRAMEWORK_COMMON_PATH_DIRECTORY_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>

#include <cr/common/exception.h>

namespace cr
{
    namespace framework
    {

        /** 目录树异常类型 */
        class PathDirectoryException : public cr::Exception
        {
        public:

            /** 错误码 */
            enum Category
            {
                /** 无效的路径 */
                INVALID_PATH = 1,
            };

            /**
            * 构造函数
            * @see Category
            * @param category 错误类别
            * @param message 错误消息
            */
            PathDirectoryException(int category, std::string message);

            /**
            * 获取错误类别编码
            * @see Category
            * @param 错误类别编码
            */
            int getCategoryCode() const;

        private:

            // 错误类别
            int category_;
        };

        /**
         * Key-Value的数据结构
         * 1、具有类文件系统的层次结构
         * 2、节点带版本号，父节点继承子节点的版本号
         * 3、节点由字母、数字、'/'组成，'/'不能连续，第一个字符必须是'/','/'后面必须有字符
         */
        class PathDirectory
        {
        public:

            /** 构造函数 */
            PathDirectory();

            /** 
             * 拷贝构造函数
             * @param other 源对象
             */
            PathDirectory(const PathDirectory& other);

            /** 
             * 移动构造函数
             * @param other 源对象
             */
            PathDirectory(PathDirectory&& other);

            /** 析构函数 */
            ~PathDirectory();

            /** 
             * 拷贝赋值函数
             * @param other 源对象
             */
            PathDirectory& operator=(const PathDirectory& other);

            /** 
             * 移动赋值函数
             * @param other 源对象
             */
            PathDirectory& operator=(PathDirectory&& other);

            /**
             * 交换对象
             * @param other 交换对象
             */
            void swap(PathDirectory& other);

            /**
             * 验证path的有效性
             * 节点由字母、数字、'/'组成，'/'不能连续，第一个字符必须是'/','/'后面必须有字符
             * @param path 待验证的路径
             * @return True 有效, false其它
             */
            static bool checkValidPath(const std::string& path);

            /**
             * 获取根的子节点数目
             */
            std::size_t getRootChildrenCount() const;

        private:

            // 实现
            class Impl;
            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif
#ifndef BiNARY_H
#define BiNARY_H
#include<vector>
#include<iostream>
#include<string>
#include <memory>

namespace BwtFS::Node{
    class Binary{
        public:
            // 构造函数，无参数
            Binary();
            // 构造函数，参数为size_t类型
            Binary(const size_t size);
            // 构造函数，参数为std::string类型
            Binary(const std::string& data);
            // 构造函数，参数为std::vector<std::byte>类型
            Binary(const std::vector<std::byte>& data);
            // 构造函数，参数为std::byte*类型和size_t类型
            Binary(const std::byte* data, const size_t size);
            // 构造函数，参数为Binary类型
            Binary(const Binary& other);
            // 赋值运算符，参数为Binary类型
            Binary& operator=(const Binary& other);
            // 移动构造函数，参数为Binary类型
            Binary(Binary&& other) = default;
            // 移动赋值运算符，参数为Binary类型
            Binary& operator=(Binary&& other);
            // 等于运算符，参数为Binary类型
            bool operator==(const Binary& other) const;
            // 不等于运算符，参数为Binary类型
            bool operator!=(const Binary& other) const;
            // 析构函数
            ~Binary() = default;

            // 下标运算符，参数为size_t类型
            std::byte& operator[](const size_t index) const;
            // 加法运算符，参数为Binary类型
            Binary& operator+(const Binary& other);

            // 读取数据，参数为size_t类型
            virtual std::byte* read(const size_t index, const size_t size) const;
            // 读取数据，参数为size_t类型
            virtual std::byte* read(const size_t index) const;
            // 读取数据，无参数
            virtual std::byte* read() const;

            // 获取数据，参数为size_t类型
            virtual std::byte get(const size_t index) const;
            
            // 写入数据，参数为size_t类型和std::byte类型
            virtual void set(const size_t index, const std::byte data);
            
            // 写入数据，参数为size_t类型和std::byte*类型
            virtual bool write(const size_t index, const size_t size, const std::byte* data);
            // 写入数据，参数为size_t类型和std::vector<std::byte>类型
            virtual bool write(const size_t index, const size_t size, std::vector<std::byte>& data);
            // 写入数据，参数为size_t类型和std::byte*类型
            virtual bool write(const size_t index, const std::byte* data);
            // 写入数据，参数为size_t类型和std::vector<std::byte>类型
            virtual bool write(const size_t index, const std::vector<std::byte>& data);
     

            // 写入数据，参数为std::byte*类型
            virtual bool write(const std::byte* data);
            // 写入数据，参数为std::vector<std::byte>类型
            virtual bool write(const std::vector<std::byte>& data);

            // 追加数据，参数为size_t类型和std::byte*类型
            virtual Binary& append(const size_t size, const std::byte* data);
            // 追加数据，参数为std::vector<std::byte>类型
            virtual Binary& append(const std::vector<std::byte>& data);

            // 清空数据
            virtual Binary& clear();

            // 获取数据大小
            virtual size_t size() const;
            // 调整数据大小，参数为size_t类型
            virtual Binary& resize(const size_t size);
            // 将数据转换为字符串，参数为size_t类型
            virtual std::string to_string(const size_t index, const size_t size) const;
            // 将数据转换为字符串，无参数
            virtual std::string to_string() const;

            // 将std::byte*类型的数据转换为字符串
            const static std::string BINARY_TO_STRING(const std::byte* data, const size_t size);
            // 将字符串转换为std::byte*类型的数据
            const static std::vector<std::byte> STRING_TO_BINARY(const std::string& data);
        private:

            // 二进制数据数组
            std::shared_ptr<std::vector<std::byte>>  binary_array;
    };
}
#endif
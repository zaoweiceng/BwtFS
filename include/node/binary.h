#ifndef BiNARY_H
#define BiNARY_H
#include <vector>
#include <iostream>
#include <string>
#include <memory>

namespace BwtFS::Node{
    enum StringType{
        BINARY, // 二进制
        ASCII,  // ASCII
        BASE64  // Base64
    };
    /*
    * 二进制数据类
    * 用于二进制数据的读写操作
    * @author: zaoweiceng
    * @data: 2025-03-18
    */
    class Binary{
        friend BwtFS::Node::Binary& operator<<(BwtFS::Node::Binary&& dest, BwtFS::Node::Binary&& src); 
        friend BwtFS::Node::Binary& operator<<(BwtFS::Node::Binary& dest, BwtFS::Node::Binary& src); 
        public:
        //----------- 构造函数和析构函数 ------------
            // 构造函数，无参数
            explicit Binary();
            // 构造函数，参数为size_t类型
            explicit Binary(const size_t size);
            // 构造函数，参数为std::string类型
            Binary(const std::string& data, StringType type = StringType::BINARY);
            // 构造函数，参数为std::vector<std::byte>类型
            Binary(const std::vector<std::byte>& data);
            // 构造函数，参数为std::shared_ptr<std::vector<std::byte>>类型
            Binary(std::shared_ptr<std::vector<std::byte>> data);
            // 构造函数，参数为std::byte*类型和size_t类型
            Binary(const std::byte* data, const size_t size);
            // 构造函数，参数为Binary类型
            Binary(const Binary& other);
            // 赋值运算符，参数为Binary类型
            Binary& operator=(const Binary& other);
            // 移动构造函数，参数为Binary类型
            Binary(Binary&& other);
            // 移动赋值运算符，参数为Binary类型
            Binary& operator=(Binary&& other) ;
            Binary& operator+=(Binary&& other);
            // 析构函数
            ~Binary() = default;
        // ----------- 运算符重载 ------------
            // 等于运算符，参数为Binary类型
            bool operator==(const Binary& other) const;
            // 不等于运算符，参数为Binary类型
            bool operator!=(const Binary& other) const;
            // 下标运算符，参数为size_t类型
            std::byte& operator[](const size_t index) const;
            // 加法运算符，参数为Binary类型
            Binary operator+(const Binary& other);
            // TODO: &运算符
            Binary operator^(const Binary& other);
        
        // ----------- 成员函数 ------------
        
        // ------------ 读数据 -------------
            // 读取数据，参数为size_t类型
            virtual std::vector<std::byte> read(const size_t index, const size_t size) const;
            // 读取数据，参数为size_t类型
            virtual std::vector<std::byte> read(const size_t index) const;
            // 读取数据，无参数
            virtual std::vector<std::byte> read() const;
            // 获取数据，参数为size_t类型
            virtual std::byte get(const size_t index) const;
            
        // ------------ 写数据 -------------
            // 写入数据，参数为size_t类型和std::byte类型
            virtual void set(const size_t index, const std::byte data);
            // 写入数据，参数为size_t类型和std::byte*类型
            virtual bool write(const size_t index, const size_t size, const std::byte* data);
            // 写入数据，参数为size_t类型和std::vector<std::byte>类型
            virtual bool write(const size_t index, const size_t size, std::vector<std::byte>& data);
            // 写入数据，参数为size_t类型和std::vector<std::byte>类型
            virtual bool write(const size_t index, const std::vector<std::byte>& data);
            // 写入数据，参数为std::byte*类型
            virtual bool write(const std::byte* data, size_t size);
            // 写入数据，参数为std::vector<std::byte>类型
            virtual bool write(const std::vector<std::byte>& data);
        // ------------ 追加写 -------------
            // 追加数据，参数为size_t类型和std::byte*类型
            virtual Binary& append(const size_t size, const std::byte* data);
            // 追加数据，参数为std::vector<std::byte>类型
            virtual Binary& append(const std::vector<std::byte>& data);

        // ------------ 其他操作 -------------
            // 清空数据
            virtual Binary& clear();
            // 获取数据大小
            virtual size_t size() const;
            // 调整数据大小，参数为size_t类型
            virtual Binary& resize(const size_t size);
            // 将数据转换为字符串，参数为size_t类型
            virtual std::string to_hex_string(const size_t index, const size_t size) const;
            // 将数据转换为字符串，无参数
            virtual std::string to_hex_string() const;
            //将数据转换为Ascll字符串，参数为size_t类型
            virtual std::string to_ascll_string(const size_t index, const size_t size) const;
            // 将数据转换为Ascll字符串，无参数
            virtual std::string to_ascll_string() const;
            // 将数据转换为Base64字符串，无参数
            virtual std::string to_base64_string() const;
            // 判断数据是否为空
            virtual bool empty() const;
            // 判断数据指针是否为空
            virtual bool is_null() const;
            // 获取数据指针
            virtual std::byte* data();

        // ----------- 静态函数 ------------
            // 将std::byte*类型的数据转换为字符串
            const static std::string BINARY_TO_STRING(const std::vector<std::byte>& data, const size_t size);
            // 将字符串转换为std::vector<std::byte>类型的数据
            const static std::vector<std::byte> STRING_TO_BINARY(const std::string& data);
            // 将std::byte*类型的数据转换为Ascll字符串
            const static std::string BINARY_TO_ASCll(const std::vector<std::byte>& data, const size_t size);
            // 将Ascll字符串转换为std::vector<std::byte>类型的数据
            const static std::vector<std::byte> ASCll_TO_BINARY(const std::string& data);
            // 将std::vector<std::byte>类型的数据转换为Base64字符串
            const static std::string BINARY_TO_BASE64(const std::vector<std::byte>& data);
            // 将Base64字符串转换为std::vector<std::byte>类型的数据
            const static std::vector<std::byte> BASE64_TO_BINARY(const std::string& data);
            // 将多个Binary对象连接起来
            const static Binary contact(std::initializer_list<Binary>&& args);
        
        private:
        // ----------- 成员变量 ------------
            // 二进制数据数组
            std::shared_ptr<std::vector<std::byte>>  binary_array;
    };

    // 重载<<运算符
    // 要用 operator<< 进行合并，必须定义为非成员函数，否则会因为隐式 this 参数导致编译错误。
    Binary& operator<<(Binary&& dest, Binary&& src);
    // 重载<<运算符
    // 要用 operator<< 进行合并，必须定义为非成员函数，否则会因为隐式 this 参数导致编译错误。
    Binary& operator<<(Binary& dest, Binary& src);
}
#endif
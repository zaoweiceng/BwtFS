#include "config.h"
#include "node/binary.h"
#include <iostream>
#include <string>
#include <cstddef>
#include <sstream>
#include <vector>
#include <iomanip>
#include <memory>
#include <array>
#include <utility>
#include <algorithm>
#include <iterator>
#include "util/log.h"

using BwtFS::Util::Logger;

BwtFS::Node::Binary::Binary(){
    this->binary_array = std::make_shared<std::vector<std::byte>>(0); 
}

BwtFS::Node::Binary::Binary(const std::string& data, BwtFS::Node::StringType type){
    if(type == BwtFS::Node::StringType::BINARY){
        this->binary_array = std::make_shared<std::vector<std::byte>>(STRING_TO_BINARY(data));
    }else if (type == BwtFS::Node::StringType::ASCII){
        this->binary_array = std::make_shared<std::vector<std::byte>>(ASCll_TO_BINARY(data));
    }else if (type == StringType::BASE64){
        this->binary_array = std::make_shared<std::vector<std::byte>>(BASE64_TO_BINARY(data));
    }
}

BwtFS::Node::Binary::Binary(const std::vector<std::byte>& data){
    this->binary_array = std::make_shared<std::vector<std::byte>>(data);
}

BwtFS::Node::Binary::Binary(std::shared_ptr<std::vector<std::byte>> data){
    this->binary_array = data;
}

BwtFS::Node::Binary::Binary(const std::byte* data, const size_t size){
    this->binary_array = std::make_shared<std::vector<std::byte>>(data, data + size);
}
BwtFS::Node::Binary::Binary(const Binary& other){
    if (other.binary_array == nullptr){
        LOG_ERROR << "constructor: Other Binary array is null";
        throw std::runtime_error("constructor: Other Binary array is null");
    }
    this->binary_array = other.binary_array;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(const Binary& other){
    if (this == &other)
        return *this;
    if (other.binary_array == nullptr){
        LOG_ERROR << "operator=: Other Binary array is null";
        throw std::runtime_error("operator=: Other Binary array is null");
    }
    this->binary_array = other.binary_array; 
    return *this;
}

BwtFS::Node::Binary::Binary(const size_t size){
    this->binary_array = std::make_shared<std::vector<std::byte>>(size);
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(Binary&& other){
    if (this != &other){
        if(other.binary_array == nullptr){
            LOG_ERROR << "operator=: Other Binary array is null";
            throw std::runtime_error("operator=: Other Binary array is null");
        }
        this->binary_array = other.binary_array;
        other.binary_array = nullptr;
    }
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator+=(Binary&& other){
    if (this != &other){
        if(other.binary_array == nullptr){
            LOG_ERROR << "operator+=: Other Binary array is null";
            throw std::runtime_error("operator+=: Other Binary array is null");
        }
        this->binary_array->insert(this->binary_array->end(), other.binary_array->begin(), other.binary_array->end());
        other.binary_array = nullptr;
    }
    return *this;
}

// std::make_move_iterator 是一种高效的方式，可以移动大型对象而不会产生额外的内存拷贝开销。
BwtFS::Node::Binary &BwtFS::Node::operator<<(Binary &&dest, Binary &&src){
    if (dest.binary_array == nullptr || src.binary_array == nullptr){
        LOG_ERROR << "operator<<: Binary array is null";
        throw std::runtime_error("operator<<: Binary array is null");
    }
    dest.binary_array->insert(dest.binary_array->end(), std::make_move_iterator(src.binary_array->begin()), std::make_move_iterator(src.binary_array->end()));
    return dest;
}

BwtFS::Node::Binary &BwtFS::Node::operator<<(Binary &dest, Binary &src){
    if (dest.binary_array == nullptr || src.binary_array == nullptr){
        LOG_ERROR << "operator<<: Binary array is null";
        throw std::runtime_error("operator<<: Binary array is null");
    }
    dest.binary_array->insert(dest.binary_array->end(), std::make_move_iterator(src.binary_array->begin()), std::make_move_iterator(src.binary_array->end()));
    return dest;
}

std::byte& BwtFS::Node::Binary::operator[](const size_t index) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "operator[]: Binary array is null";
        throw std::runtime_error("operator[]: Binary array is null");
    }
    if (index >= this->binary_array->size()){
        LOG_ERROR << "operator[]: Index out of range";
        throw std::runtime_error("operator[]: Index out of range");
    }
    return this->binary_array->at(index);
}

BwtFS::Node::Binary BwtFS::Node::Binary::operator+(const Binary& other){
    if (other.binary_array == nullptr){
        LOG_ERROR << "operator+: Other Binary array is null";
        throw std::runtime_error("operator+: Other Binary array is null");
    }
    if (this->binary_array == nullptr){
        LOG_ERROR << "operator+: Binary array is null";
        throw std::runtime_error("operator+: Binary array is null");
    }
    std::shared_ptr<std::vector<std::byte>> new_array = std::make_shared<std::vector<std::byte>>(this->binary_array->size() + other.binary_array->size());
    std::copy(this->binary_array->begin(), this->binary_array->end(), new_array->begin());
    std::copy(other.binary_array->begin(), other.binary_array->end(), new_array->begin() + this->binary_array->size());
    return BwtFS::Node::Binary(new_array->data(), new_array->size());
}

bool BwtFS::Node::Binary::operator==(const Binary& other) const{
    if (this->binary_array == nullptr || other.binary_array == nullptr)
        return false;
    return this->to_hex_string() == other.to_hex_string();
}

bool BwtFS::Node::Binary::operator!=(const Binary& other) const{
    if (this->binary_array == nullptr || other.binary_array == nullptr)
        return true;
    return this->to_hex_string() != other.to_hex_string();
}

std::vector<std::byte> xorVectors(const std::shared_ptr<std::vector<std::byte>>& v1, const std::shared_ptr<std::vector<std::byte>>& v2){
    if (v1->size() < v2->size()) {
        return xorVectors(v2, v1);
    }
    std::vector<std::byte> result;
    int j = 0;
    for (size_t i = 0; i < v1->size(); i++, j++) {
        if (j == v2->size()) {
            j = 0;
        }
        result.push_back(v1->at(i) ^ v2->at(j));
    }
    return result;
}
BwtFS::Node::Binary BwtFS::Node::Binary::operator^(const Binary& other){
    if (this->binary_array == nullptr) {
        LOG_ERROR << "operator^: Binary array is null";
        throw std::runtime_error("operator^: Binary array is null");
    }
    if (other.binary_array == nullptr) {
        LOG_ERROR << "operator^: Other Binary array is null";
        throw std::runtime_error("operator^: Other Binary array is null");
    }
    return xorVectors(this->binary_array, other.binary_array);
}

std::vector<std::byte> BwtFS::Node::Binary::read(const size_t index, const size_t size) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::read: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::read: Binary array is null");
    }
    size_t read_size = size;
    if (index + size > this->binary_array->size())
        read_size = this->binary_array->size() - index;
    if (read_size <= 0)
        return {};
    return std::vector<std::byte>(this->binary_array->begin() + index, this->binary_array->begin() + index + read_size);
}

std::vector<std::byte> BwtFS::Node::Binary::read(const size_t index) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::read: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::read: Binary array is null");
    }
    return read(index, this->binary_array->size() - index);
}

std::vector<std::byte> BwtFS::Node::Binary::read() const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::read: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::read: Binary array is null");
    }
    return read(0, this->binary_array->size());
}

std::byte BwtFS::Node::Binary::get(const size_t index) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::get: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::get: Binary array is null");
    }
    if (index >= this->binary_array->size()){
        LOG_ERROR << "BwtFS::Node::Binary::get: Index out of range";
        throw std::runtime_error("BwtFS::Node::Binary::get: Index out of range");
    }
    return this->binary_array->at(index);
}

void BwtFS::Node::Binary::set(const size_t index, const std::byte data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::set: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::set: Binary array is null");
    }
    if (index >= this->binary_array->size()){
        LOG_ERROR << "BwtFS::Node::Binary::set: Index out of range";
        throw std::runtime_error("BwtFS::Node::Binary::set: Index out of range");
    }
    if (index < 0){
        LOG_ERROR << "BwtFS::Node::Binary::set: Index out of range";
        throw std::runtime_error("BwtFS::Node::Binary::set: Index out of range");
    }
    this->binary_array->at(index) = data;
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, const std::byte* data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::write: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::write: Binary array is null");
    }
    if (index + size > this->binary_array->size() || index < 0 || size < 0)
        return false;
    std::copy(data, data + size, this->binary_array->begin() + index);
    return true;
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, std::vector<std::byte>& data){
    return write(index, size, data.data());
}

bool BwtFS::Node::Binary::write(const size_t index, const std::byte* data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::write: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::write: Binary array is null");
    }
    return write(index, this->binary_array->size() - index, data);
}

bool BwtFS::Node::Binary::write(const size_t index, const std::vector<std::byte>& data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::write: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::write: Binary array is null");
    }
    return write(index, this->binary_array->size() - index, data.data());
}

bool BwtFS::Node::Binary::write(const std::byte* data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::write: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::write: Binary array is null");
    }
    return write(0, this->binary_array->size(), data);
}

bool BwtFS::Node::Binary::write(const std::vector<std::byte>& data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::write: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::write: Binary array is null");
    }
    return write(0, this->binary_array->size(), data.data());
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const size_t size, const std::byte* data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::append: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::append: Binary array is null");
    }
    this->binary_array->insert(this->binary_array->end(), data, data + size);
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const std::vector<std::byte>& data){
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::append: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::append: Binary array is null");
    }
    this->binary_array->insert(this->binary_array->end(), data.begin(), data.end());
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::clear(){
    if (this->binary_array == nullptr){
        LOG_WARNING << "BwtFS::Node::Binary::clear: Binary array is null";
    }
    this->binary_array->clear();
    return *this;
}

size_t BwtFS::Node::Binary::size() const{
    if (this->binary_array == nullptr){
        LOG_WARNING << "BwtFS::Node::Binary::size: Binary array is null";
        return 0;
    }
    return this->binary_array->size();
}

std::string byteToHex(std::byte b) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return ss.str();
}

std::string byteArrayToHexString(const std::vector<std::byte>& data, size_t size) {
    std::string hexString;
    for (size_t i = 0; i < size; i++) {
        hexString += byteToHex(data[i]);
    }
    return hexString;
}

std::byte hexCharToByte(char c) {
    if (c >= '0' && c <= '9') return static_cast<std::byte>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<std::byte>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<std::byte>(c - 'A' + 10);
    return static_cast<std::byte>(0);
}

std::vector<std::byte> hexStringToByteArray(const std::string& hexString) {
    if (hexString.length() % 2 != 0) {
        return {}; // 返回空数组，因为十六进制字符串长度必须是偶数
    }

    std::vector<std::byte> byteArray;
    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::byte high = hexCharToByte(hexString[i]);
        std::byte low = hexCharToByte(hexString[i + 1]);
        byteArray.push_back(static_cast<std::byte>(static_cast<unsigned char>(high) << 4 | static_cast<unsigned char>(low)));
    }
    return byteArray;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::resize(const size_t size){
    if (this->binary_array == nullptr){
        this->binary_array = std::make_shared<std::vector<std::byte>>(size);
        return *this;
    }
    if (size <= this->size())
        return *this;
    this->binary_array->resize(size);
    return *this;
}

const std::string BwtFS::Node::Binary::BINARY_TO_STRING(const std::vector<std::byte>& data, const size_t size){
    return byteArrayToHexString(data, size);
}
const std::vector<std::byte> BwtFS::Node::Binary::STRING_TO_BINARY(const std::string& data){
    return hexStringToByteArray(data);
}

std::string BwtFS::Node::Binary::to_hex_string(const size_t index, const size_t size) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::to_string: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::to_string: Binary array is null");
    }
    if (index + size > this->binary_array->size())
        return BINARY_TO_STRING(*this->binary_array, this->binary_array->size());
    if (index < 0 || size < 0)
        return BINARY_TO_STRING(*this->binary_array, this->binary_array->size());
    return BINARY_TO_STRING(*this->binary_array, size);
}

std::string BwtFS::Node::Binary::to_hex_string() const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::to_string: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::to_string: Binary array is null");
    }
    if (this->binary_array->size() == 0)
        return "";
    return BINARY_TO_STRING(*this->binary_array, this->binary_array->size());
}

const std::string BwtFS::Node::Binary::BINARY_TO_ASCll(const std::vector<std::byte>& data, const size_t size){
    std::string str;
    for (size_t i = 0; i < size; i++){
        str += static_cast<char>(data[i]);
    }
    return str;
}

const std::vector<std::byte> BwtFS::Node::Binary::ASCll_TO_BINARY(const std::string& data){
    std::vector<std::byte> binary;
    for (size_t i = 0; i < data.size(); i++){
        binary.push_back(static_cast<std::byte>(data[i]));
    }
    return binary;
}

std::string BwtFS::Node::Binary::to_ascll_string(const size_t index, const size_t size) const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::to_ascll_string: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::to_ascll_string: Binary array is null");
    }
    if (index + size > this->binary_array->size())
        return BINARY_TO_ASCll(*this->binary_array, this->binary_array->size());
    return BINARY_TO_ASCll(*this->binary_array, size);
}

std::string BwtFS::Node::Binary::to_ascll_string() const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::to_ascll_string: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::to_ascll_string: Binary array is null");
    }
    if (this->binary_array->size() == 0)
        return "";
    return BINARY_TO_ASCll(*this->binary_array, this->binary_array->size());
}

std::string base64_encode(const std::vector<std::byte>& input) {
    static constexpr char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    const unsigned char* data = reinterpret_cast<const unsigned char*>(input.data());
    size_t input_len = input.size();
    std::string output;
    output.reserve(4 * ((input_len + 2) / 3)); // 预分配空间
    for (size_t i = 0; i < input_len; i += 3) {
        unsigned char a = data[i];
        unsigned char b = (i + 1 < input_len) ? data[i + 1] : 0;
        unsigned char c = (i + 2 < input_len) ? data[i + 2] : 0;

        unsigned char group1 = (a >> 2) & 0x3F;
        unsigned char group2 = ((a & 0x03) << 4) | ((b >> 4) & 0x0F);
        unsigned char group3 = ((b & 0x0F) << 2) | ((c >> 6) & 0x03);
        unsigned char group4 = c & 0x3F;

        output.push_back(base64_chars[group1]);
        output.push_back(base64_chars[group2]);
        output.push_back(base64_chars[group3]);
        output.push_back(base64_chars[group4]);
    }
    // 处理填充
    size_t mod = input_len % 3;
    if (mod == 1) {
        // 替换最后两个字符为 '=='
        output[output.size() - 2] = '=';
        output[output.size() - 1] = '=';
    } else if (mod == 2) {
        // 替换最后一个字符为 '='
        output[output.size() - 1] = '=';
    }
    return output;
}

std::vector<std::byte> base64_to_bytes(const std::string& input) {
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    // 创建解码表（字符到6位值）
    std::array<int, 256> decode_table{};
    for (auto& elem : decode_table) elem = -1;
    for (int i = 0; i < 64; ++i) {
        decode_table[static_cast<unsigned char>(base64_chars[i])] = i;
    }

    const size_t len = input.size();
    if (len % 4 != 0) {
        LOG_ERROR << "base64_to_bytes: Base64 string length must be a multiple of 4";
        throw std::invalid_argument("base64_to_bytes: Base64 string length must be a multiple of 4");
    }

    // 计算填充符数量（0/1/2）
    size_t padding = 0;
    if (len > 0 && input[len - 1] == '=') padding++;
    if (len > 1 && input[len - 2] == '=') padding++;

    std::vector<std::byte> result;
    result.reserve((len / 4) * 3 - padding); // 预分配内存优化

    for (size_t i = 0; i < len; i += 4) {
        // 提取四字符组中的6位值
        uint32_t sextet_a = decode_table[static_cast<unsigned char>(input[i])];
        uint32_t sextet_b = decode_table[static_cast<unsigned char>(input[i+1])];
        uint32_t sextet_c = 0, sextet_d = 0;

        // 处理第三个字符（允许为=）
        if (input[i+2] != '=') {
            sextet_c = decode_table[static_cast<unsigned char>(input[i+2])];
            if (sextet_c == -1){
                LOG_ERROR << "base64_to_bytes: Invalid character in Base64 string";
                throw std::invalid_argument("base64_to_bytes: Invalid character in Base64 string");
            }
        }

        // 处理第四个字符（允许为=）
        if (input[i+3] != '=') {
            sextet_d = decode_table[static_cast<unsigned char>(input[i+3])];
            if (sextet_d == -1){
                LOG_ERROR << "base64_to_bytes: Invalid character in Base64 string";
                throw std::invalid_argument("base64_to_bytes: Invalid character in Base64 string");
            }
        }

        if (sextet_a == -1 || sextet_b == -1) {
            LOG_ERROR << "base64_to_bytes: Invalid character in Base64 string";
            throw std::invalid_argument("base64_to_bytes: Invalid character in Base64 string");
        }

        // 组合四字符为24位整数
        const uint32_t triplet = 
            (sextet_a << 18) | (sextet_b << 12) | 
            (sextet_c << 6)  | sextet_d;

        // 提取三个字节
        result.push_back(static_cast<std::byte>((triplet >> 16) & 0xFF));
        if (input[i+2] != '=') {  // 非填充情况
            result.push_back(static_cast<std::byte>((triplet >> 8) & 0xFF));
            if (input[i+3] != '=') {
                result.push_back(static_cast<std::byte>(triplet & 0xFF));
            }
        } else {
            // 第三个字符是=，第四个必须也是=
            if (input[i+3] != '=') {
                LOG_ERROR << "base64_to_bytes: Invalid padding with '='";
                throw std::invalid_argument("base64_to_bytes: Invalid padding with '='");
            }
        }
    }
    return result;
}



const std::string BwtFS::Node::Binary::BINARY_TO_BASE64(const std::vector<std::byte>& data){
    return base64_encode(data);
}

const std::vector<std::byte> BwtFS::Node::Binary::BASE64_TO_BINARY(const std::string& data){
    return base64_to_bytes(data);
}

std::string BwtFS::Node::Binary::to_base64_string() const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::to_base64_string: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::to_base64_string: Binary array is null");
    }
    if (this->binary_array->size() == 0)
        return "";
    return BINARY_TO_BASE64(*this->binary_array);
}

const BwtFS::Node::Binary BwtFS::Node::Binary::contact(std::initializer_list<Binary>&& args){
    BwtFS::Node::Binary binary(0);
    for (auto arg : args){
        if (arg.binary_array == nullptr){
            LOG_ERROR << "BwtFS::Node::Binary::contact: Binary array is null";
            throw std::runtime_error("BwtFS::Node::Binary::contact: Binary array is null");
        }
        binary += std::move(arg);
    }
    return binary;
}

bool BwtFS::Node::Binary::empty() const{
    if (this->binary_array == nullptr){
        LOG_ERROR << "BwtFS::Node::Binary::empty: Binary array is null";
        throw std::runtime_error("BwtFS::Node::Binary::empty: Binary array is null");
    }
    return this->binary_array->empty();
}

bool BwtFS::Node::Binary::is_null() const{
    return this->binary_array == nullptr;
}
#include "config.h"
#include"node/binary.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <memory>

BwtFS::Node::Binary::Binary(){
    this->binary_array = std::make_shared<std::vector<std::byte>>(BwtFS::BLOCK_SIZE); 
}

BwtFS::Node::Binary::Binary(const std::string& data){
    this->binary_array = std::make_shared<std::vector<std::byte>>(STRING_TO_BINARY(data));

}

BwtFS::Node::Binary::Binary(const std::vector<std::byte>& data){
    this->binary_array = std::make_shared<std::vector<std::byte>>(data);
}

BwtFS::Node::Binary::Binary(const std::byte* data, const size_t size){
    this->binary_array = std::make_shared<std::vector<std::byte>>(data, data + size);
}
BwtFS::Node::Binary::Binary(const Binary& other){
    this->binary_array = other.binary_array;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(const Binary& other){
    this->binary_array = other.binary_array; 
    return *this;
}

BwtFS::Node::Binary::Binary(const size_t size){
    this->binary_array = std::make_shared<std::vector<std::byte>>(size);
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(Binary&& other){
    this->binary_array = std::move(other.binary_array);
    return *this;
}

std::byte& BwtFS::Node::Binary::operator[](const size_t index) const{
    return this->binary_array->at(index);
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator+(const Binary& other){
    std::shared_ptr<std::vector<std::byte>> new_array = std::make_shared<std::vector<std::byte>>(this->binary_array->size() + other.binary_array->size());
    std::copy(this->binary_array->begin(), this->binary_array->end(), new_array->begin());
    std::copy(other.binary_array->begin(), other.binary_array->end(), new_array->begin() + this->binary_array->size());
    BwtFS::Node::Binary* new_binary = new BwtFS::Node::Binary(new_array->data(), new_array->size());
    return *new_binary;
}

bool BwtFS::Node::Binary::operator==(const Binary& other) const{
    return this->to_string() == other.to_string();
}

bool BwtFS::Node::Binary::operator!=(const Binary& other) const{
    return this->to_string() == other.to_string();
}

std::byte* BwtFS::Node::Binary::read(const size_t index, const size_t size) const{
    size_t read_size = size;
    if (index + size > this->binary_array->size())
        read_size = this->binary_array->size() - index;
    if (read_size <= 0)
        return nullptr;
    std::byte* data = new std::byte[read_size];
    std::copy(this->binary_array->begin() + index, this->binary_array->begin() + index + size, data);
    return data;
}

std::byte* BwtFS::Node::Binary::read(const size_t index) const{
    return read(index, this->binary_array->size() - index);
}

std::byte* BwtFS::Node::Binary::read() const{
    return read(0, this->binary_array->size());
}

std::byte BwtFS::Node::Binary::get(const size_t index) const{
    return this->binary_array->at(index);
}

void BwtFS::Node::Binary::set(const size_t index, const std::byte data){
    this->binary_array->at(index) = data;
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, const std::byte* data){
    if (index + size > this->binary_array->size() || index < 0 || size < 0)
        return false;
    std::copy(data, data + size, this->binary_array->begin() + index);
    return true;
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, std::vector<std::byte>& data){
    return write(index, size, data.data());
}

bool BwtFS::Node::Binary::write(const size_t index, const std::byte* data){
    return write(index, this->binary_array->size() - index, data);
}

bool BwtFS::Node::Binary::write(const size_t index, const std::vector<std::byte>& data){
    return write(index, this->binary_array->size() - index, data.data());
}

bool BwtFS::Node::Binary::write(const std::byte* data){
    return write(0, this->binary_array->size(), data);
}

bool BwtFS::Node::Binary::write(const std::vector<std::byte>& data){
    return write(0, this->binary_array->size(), data.data());
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const size_t size, const std::byte* data){
    this->binary_array->insert(this->binary_array->end(), data, data + size);
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const std::vector<std::byte>& data){
    this->binary_array->insert(this->binary_array->end(), data.begin(), data.end());
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::clear(){
    this->binary_array->clear();
    return *this;
}

size_t BwtFS::Node::Binary::size() const{
    return this->binary_array->size();
}

std::string byteToHex(std::byte b) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return ss.str();
}

std::string byteArrayToHexString(const std::byte* bytes, size_t size) {
    std::string hexString;
    for (size_t i = 0; i < size; i++) {
        hexString += byteToHex(bytes[i]);
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
    this->binary_array->resize(size);
    return *this;
}

const std::string BwtFS::Node::Binary::BINARY_TO_STRING(const std::byte* data, const size_t size){
    return byteArrayToHexString(data, size);
}
const std::vector<std::byte> BwtFS::Node::Binary::STRING_TO_BINARY(const std::string& data){
    return hexStringToByteArray(data);
}

std::string BwtFS::Node::Binary::to_string(const size_t index, const size_t size) const{
    return BINARY_TO_STRING(this->binary_array->data() + index, size);
}

std::string BwtFS::Node::Binary::to_string() const{
    return BINARY_TO_STRING(this->binary_array->data(), this->binary_array->size());
}


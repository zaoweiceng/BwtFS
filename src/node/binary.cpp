#include"config.h"
#include"node/binary.h"
#include<iostream>

BwtFS::Node::Binary::Binary(){
    binary_array = std::make_shared<std::vector<std::byte>>(BLOCK_SIZE);
}

BwtFS::Node::Binary::Binary(const Binary& other){
    binary_array = other.binary_array;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(const Binary& other){
    binary_array = other.binary_array; 
    return *this;
}

BwtFS::Node::Binary::Binary(const size_t size){
    binary_array = std::make_shared<std::vector<std::byte>>(size);
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator=(Binary&& other){
    binary_array = std::move(other.binary_array);
    return *this;
}

std::byte& BwtFS::Node::Binary::operator[](const size_t index) const{
    return binary_array->at(index);
}

BwtFS::Node::Binary& BwtFS::Node::Binary::operator+(const Binary& other){
    std::shared_ptr<std::vector<std::byte>> new_array = std::make_shared<std::vector<std::byte>>(binary_array->size() + other.binary_array->size());
    std::copy(binary_array->begin(), binary_array->end(), new_array->begin());
    std::copy(other.binary_array->begin(), other.binary_array->end(), new_array->begin() + binary_array->size());
    binary_array = new_array;
    return *this;
}

bool BwtFS::Node::Binary::operator==(const Binary& other) const{
    return binary_array == other.binary_array;
}

bool BwtFS::Node::Binary::operator!=(const Binary& other) const{
    return binary_array != other.binary_array;
}

std::byte* BwtFS::Node::Binary::read(const size_t index, const size_t size) const{
    size_t read_size = size;
    if (index + size > binary_array->size())
        read_size = binary_array->size() - index;
    if (read_size <= 0)
        return nullptr;
    std::byte* data = new std::byte[read_size];
    std::copy(binary_array->begin() + index, binary_array->begin() + index + size, data);
    return data;
}

std::byte* BwtFS::Node::Binary::read(const size_t index) const{
    return read(index, binary_array->size() - index);
}

std::byte* BwtFS::Node::Binary::read() const{
    return read(0, binary_array->size());
}

std::byte BwtFS::Node::Binary::get(const size_t index) const{
    return binary_array->at(index);
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, const std::byte* data){
    if (index + size > binary_array->size() || index < 0 || size < 0)
        return false;
    std::copy(data, data + size, binary_array->begin() + index);
    return true;
}

bool BwtFS::Node::Binary::write(const size_t index, const size_t size, std::vector<std::byte>& data){
    return write(index, size, data.data());
}

bool BwtFS::Node::Binary::write(const size_t index, const std::byte* data){
    return write(index, binary_array->size() - index, data);
}

bool BwtFS::Node::Binary::write(const size_t index, const std::vector<std::byte>& data){
    return write(index, binary_array->size() - index, data.data());
}

bool BwtFS::Node::Binary::write(const std::byte* data){
    return write(0, binary_array->size(), data);
}

bool BwtFS::Node::Binary::write(const std::vector<std::byte>& data){
    return write(0, binary_array->size(), data.data());
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const size_t size, const std::byte* data){
    binary_array->insert(binary_array->end(), data, data + size);
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::append(const std::vector<std::byte>& data){
    binary_array->insert(binary_array->end(), data.begin(), data.end());
    return *this;
}

BwtFS::Node::Binary& BwtFS::Node::Binary::clear(){
    binary_array->clear();
    return *this;
}

size_t BwtFS::Node::Binary::size() const{
    return binary_array->size();
}

BwtFS::Node::Binary& BwtFS::Node::Binary::resize(const size_t size){
    binary_array->resize(size);
    return *this;
}

const std::string BwtFS::Node::Binary::BINARY_TO_STRING(const std::byte* data, const size_t size){
    return std::string(data, data + size);
}

const std::byte* BwtFS::Node::Binary::STRING_TO_BINARY(const std::string& data){
    return reinterpret_cast<const std::byte*>(data.c_str());
}

std::string BwtFS::Node::Binary::to_string(const size_t index, const size_t size) const{
    return BINARY_TO_STRING(binary_array->data() + index, size);
}

std::string BwtFS::Node::Binary::to_string() const{
    return BINARY_TO_STRING(binary_array->data(), binary_array->size());
}
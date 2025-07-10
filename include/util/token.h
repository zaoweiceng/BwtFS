#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <stdexcept>
#include <ctime>
#include <algorithm>
#include "node/binary.h"
#include "cell.h"

using BwtFS::Node::Binary;
using BwtFS::Node::StringType;

const std::string encodingChars = 
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789*-";

class Token {
    public:
        Token(size_t bitmap, uint16_t start, uint16_t length, uint16_t seed, uint8_t level)
            :bitmap_(bitmap), start_(start), length_(length), seed_(seed), level_(level){}
        std::string generate_token(){
            using BwtFS::Util::RCA;
            std::string token;
            Binary data;
            rca_ = std::time(nullptr);
            data.append(sizeof(bitmap_), reinterpret_cast<std::byte*>(&bitmap_));
            data.append(sizeof(start_), reinterpret_cast<std::byte*>(&start_));
            data.append(sizeof(length_), reinterpret_cast<std::byte*>(&length_));
            data.append(sizeof(seed_), reinterpret_cast<std::byte*>(&seed_));
            data.append(sizeof(level_), reinterpret_cast<std::byte*>(&level_));
            RCA rca = RCA(rca_, data);
            // std::cout << "rca seed: " << rca_ << std::endl;
            // std::cout << "brca: " << data.to_base64_string() << std::endl;
            rca.forward();
            // std::cout << "token: " << data.to_base64_string() << std::endl;
            std::byte* data_ptr = data.data();
            size_t data_size = data.size();
            char ch, cache = 0;
            int cache_size = 0;
            // std::cout << "data size: " << data_size << std::endl;
            for (size_t i = 0; i < data_size; i++){
                if (cache_size != 0){
                    if (cache_size == 2){
                        ch = static_cast<char>(data_ptr[i]);
                        token += encodingChars[(cache << 4) | ((ch & 0b11110000) >> 4)];
                        cache = static_cast<char>(ch & 0b00001111);
                        cache_size = 4;
                    }else if (cache_size == 4){
                        ch = static_cast<char>(data_ptr[i]);
                        token += encodingChars[(cache << 2) | ((ch & 0b11000000) >> 6)];
                        token += encodingChars[ch & 0b00111111];
                        cache = 0;
                        cache_size = 0;
                    }
                }else if (cache_size == 0){
                    ch = static_cast<char>(data_ptr[i]);
                    token += encodingChars[(ch & 0b11111100) >> 2];
                    cache = static_cast<char>(ch & 0b00000011);
                    cache_size = 2;
                }                
            }
            std::string rca_base64 = Binary().append(sizeof(rca_), reinterpret_cast<std::byte*>(&rca_)).to_base64_string();
            std::replace(rca_base64.begin(), rca_base64.end(), '=', '_');
            token = rca_base64 + token;
            // token = Binary().append(sizeof(rca_), reinterpret_cast<std::byte*>(&rca_)).to_base64_string().replace('=', '_') + token;
            // std::cout << Binary().append(sizeof(rca_), reinterpret_cast<std::byte*>(&rca_)).to_base64_string().length() << std::endl;
            return token;
        }   
        Token(std::string token){
            std::replace(token.begin(), token.end(), '_', '=');
            decode_token(token);
        }
        void decode_token(const std::string& token){
            using BwtFS::Util::RCA;
            Binary data;
            size_t token_size = token.size();
            auto rca_binary = Binary(token.substr(0, 12), StringType::BASE64);
            rca_ = reinterpret_cast<uint64_t*>(rca_binary.data())[0];
            // std::cout << "rca seed: " << rca_ << std::endl;
            // std::cout << "token: " << token.substr(12, 40) << std::endl;
            char cache = 0;
            int cache_size = 0;
            for (size_t i = 12; i < token_size; i++){
                char ch = token[i];
                if (cache_size + 6 >= 8){
                    if (cache_size == 6){
                        cache = (cache << 2) | ((encodingChars.find(ch) & 0b00110000) >> 4);
                        data.append(std::vector<std::byte>(1, static_cast<std::byte>(cache)));
                        cache = static_cast<char>(encodingChars.find(ch) & 0b00001111);
                        cache_size = 4;
                    }else if (cache_size == 4){
                        cache = (cache << 4) | ((encodingChars.find(ch) & 0b00111100) >> 2);
                        data.append(std::vector<std::byte>(1, static_cast<std::byte>(cache)));
                        cache = static_cast<char>(encodingChars.find(ch) & 0b00000011);
                        cache_size = 2;
                    }else if (cache_size == 2){
                        cache = (cache << 6) | (encodingChars.find(ch) & 0b00111111);
                        data.append(std::vector<std::byte>(1, static_cast<std::byte>(cache)));
                        cache = 0;
                        cache_size = 0;
                    }
                }else{
                    cache = encodingChars.find(ch) & 0b00111111;
                    cache_size = 6;
                }
            }
            RCA rca = RCA(rca_, data);
            // std::cout << "token: " << data.to_base64_string() << std::endl;
            rca.backward();
            auto data_ptr = data.data();
            bitmap_ = *reinterpret_cast<size_t*>(data_ptr);
            start_ = *reinterpret_cast<uint16_t*>(data_ptr + sizeof(bitmap_));
            length_ = *reinterpret_cast<uint16_t*>(data_ptr + sizeof(bitmap_) + sizeof(start_));
            seed_ = *reinterpret_cast<uint16_t*>(data_ptr + sizeof(bitmap_) + sizeof(start_) + sizeof(length_));
            level_ = *reinterpret_cast<uint8_t*>(data_ptr + sizeof(bitmap_) + sizeof(start_) + sizeof(length_) + sizeof(seed_));
        }
        size_t get_bitmap() const {
            return bitmap_;
        }
        uint16_t get_start() const {
            return start_;
        }
        uint16_t get_length() const {
            return length_;
        }
        uint16_t get_seed() const {
            return seed_;
        }
        uint8_t get_level() const {
            return level_;
        }
    private:
        size_t bitmap_;
        uint16_t start_;
        uint16_t length_;
        uint16_t seed_;
        uint8_t level_;
        uint64_t rca_;

};
#endif // __TOKEN_H__
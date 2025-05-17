#include<iostream>
#include<vector>
#include <cstddef>
#include <random>
#include "util/random.h"

namespace BwtFS::Util{
    template<typename T>
    std::vector<T> RandNumbers(int n, unsigned seed, int min, int max){
        // 生成随机数, n为生成的随机数个数，seed为随机数种子, min为随机数最小值，max为随机数最大值
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<T> distribution(min, max); // 生成[min, max]之间的随机数
        std::vector<T> v;
        for(int i = 0; i < n; i++){
            v.push_back(distribution(generator));
        }
        return v;
    }

    template std::vector<int> BwtFS::Util::RandNumbers<int>(int, unsigned, int, int);
    template std::vector<uint16_t> BwtFS::Util::RandNumbers<uint16_t>(int, unsigned, int, int);

    std::vector<std::byte> RandBytes(int n, unsigned seed, int min, int max){
        // 生成随机数, n为生成的随机数个数，seed为随机数种子, min为随机数最小值，max为随机数最大值
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distribution(min, max); // 生成[min, max]之间的随机数
        std::vector<std::byte> v;
        for(int i = 0; i < n; i++){
            v.push_back((std::byte)distribution(generator));
        }
        return v;
    }


    int RandNumber(unsigned seed, int min, int max){
        // 生成随机数, seed为随机数种子, min为随机数最小值，max为随机数最大值
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distribution(min, max); // 生成[min, max]之间的随机数
        return distribution(generator);
    }
}

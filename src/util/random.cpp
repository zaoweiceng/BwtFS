#include<iostream>
#include<vector>
#include <cstddef>
#include <random>
#include "util/random.h"

std::vector<int> BwtFS::Util::RandNumbers(int n, unsigned seed, int min, int max){
    // 生成随机数, n为生成的随机数个数，seed为随机数种子, min为随机数最小值，max为随机数最大值
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(min, max); // 生成[min, max]之间的随机数
    std::vector<int> v;
    for(int i = 0; i < n; i++){
        v.push_back(distribution(generator));
    }
    return v;
}
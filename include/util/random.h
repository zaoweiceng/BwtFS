#ifndef RANDOM_H
#define RANDOM_H
#include <vector>
namespace BwtFS::Util{
    // 生成随机数
    // n: 随机数的个数
    // seed: 随机数种子
    // min: 随机数的最小值
    // max: 随机数的最大值
    // 返回随机数的vector, int类型
    // 注意: 生成的随机数是闭区间[min, max]
    std::vector<int> RandNumbers(int n, unsigned seed, int min, int max);
};

#endif
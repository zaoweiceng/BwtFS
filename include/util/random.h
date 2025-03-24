#ifndef RANDOM_H
#define RANDOM_H
#include <vector>
namespace BwtFS::Util{
    std::vector<int> RandNumbers(int n, unsigned seed, int min, int max);
};

#endif
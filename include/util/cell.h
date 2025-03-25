# ifndef CELL_H
# define CELL_H
#include<vector>
#include <random>
#include "node/binary.h"

namespace BwtFS::Util{
    class RCA{
        /*
        * Cell类
        * 可逆细胞自动机
        * */
        public:
        RCA(unsigned seed, BwtFS::Node::Binary& binary);
            //前向迭代
            void forward();
            //反向迭代
            void backward();
            //异或操作
            static void XOR(std::byte& b);
            static void XOR_BACK(std::byte& b);
            //移位操作
            static void SHIFT(std::byte& b);
            static void SHIFT_BACK(std::byte& b);
            //FredKin逻辑门
            static void FD(std::byte& b);
            static void FD_BACK(std::byte& b);
            //Toffoli逻辑门
            static void TD(std::byte& b);
            static void TD_BACK(std::byte& b);
            //应用操作
            static void apply(std::byte& b, short operation, bool forward);


        private:
            unsigned seed;
            BwtFS::Node::Binary binary;
            std::vector<int> rule;
    };
};

# endif
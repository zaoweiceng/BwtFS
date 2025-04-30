# ifndef CELL_H
# define CELL_H
#include<vector>
#include <random>
#include "node/binary.h"

namespace BwtFS::Util{
    /*
    * Cell类
    * 可逆细胞自动机
    * 用于细胞自动机的操作
    * @author: zaoweiceng
    * @data: 2025-03-26
    * */
    class RCA{
        public:
            // 构造函数
            // seed: 细胞自动机的种子
            // binary: 细胞自动机的二进制数据
            RCA() = default;
            RCA(unsigned seed, BwtFS::Node::Binary& binary);
            RCA(RCA const& other) = delete;
            RCA& operator=(RCA const& other) = delete;
            RCA(RCA&& other) = delete;
            RCA& operator=(RCA&& other) = delete;
            ~RCA() = default;
            //前向迭代
            void forward();
            //反向迭代
            void backward();
            //异或操作，正向计算
            static void XOR(std::byte& b);
            //异或操作，反向计算
            static void XOR_BACK(std::byte& b);
            //移位操作，正向计算
            static void SHIFT(std::byte& b);
            //移位操作，反向计算
            static void SHIFT_BACK(std::byte& b);
            //FredKin逻辑门，正向计算
            static void FD(std::byte& b);
            //FredKin逻辑门，反向计算
            static void FD_BACK(std::byte& b);
            //Toffoli逻辑门, 正向计算
            static void TD(std::byte& b);
            // Toffoli逻辑门, 反向计算
            static void TD_BACK(std::byte& b);
            //应用操作
            static void apply(std::byte& b, short operation, bool forward);
            // 设置细胞自动机的种子
            void setSeed(unsigned seed);
            // 设置细胞自动机的二进制数据
            void setBinary(BwtFS::Node::Binary& binary);


        private:
            // 细胞自动机的种子
            unsigned seed;
            // 细胞自动机的二进制数据
            BwtFS::Node::Binary binary;
            // 细胞自动机的规则
            std::vector<int> rule;
    };
};

# endif
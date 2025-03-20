#ifndef BINARY_OPERATION_HPP
#define BINARY_OPERATION_HPP

namespace BwtFS::Util{
    template<typename T>
    class BitwiseOperations {
        public:
            // 提取特定位上的值
            static bool getBit(T& value, int position) {
                return (value & static_cast<std::byte>(1 << position)) == static_cast<std::byte>(1 << position);
            }
            // 设置特定位上的值为1
            static void setBit(T& value, int position) {
                value |= static_cast<std::byte>(1 << position);
            }
            // 清除特定位上的值（设置为0）
            static void clearBit(T& value, int position) {
                value &= static_cast<std::byte>(~(1 << position));
            }
            // 反转特定位上的值
            static void toggleBit(T& value, int position) {
                value ^= static_cast<std::byte>(1 << position);
            }
        };
};

#endif
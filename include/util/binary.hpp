#include <iostream>

# ifndef BITWISE_OPERATIONS_H
# define BITWISE_OPERATIONS_H
    template<typename T>
    class BitwiseOperations {
    public:
        // 提取特定位上的值
        static bool getBit(T value, int position) {
            return (value & (1 << position)) != 0;
        }

        // 设置特定位上的值为1
        static T setBit(T value, int position) {
            return value | (1 << position);
        }

        // 清除特定位上的值（设置为0）
        static T clearBit(T value, int position) {
            return value & ~(1 << position);
        }

        // 反转特定位上的值
        static T toggleBit(T value, int position) {
            return value ^ (1 << position);
        }
    };

    // int main() {
    //     unsigned char byte = 0b10101010;

    //     std::cout << "Original byte: ";
    //     for(int i=7; i>=0; --i)
    //         std::cout << BitwiseOperations<unsigned char>::getBit(byte, i);

    //     // 设置第3位为1
    //     byte = BitwiseOperations<unsigned char>::setBit(byte, 2);
        
    //     std::cout << "\nByte after setting bit 3: ";
    //     for(int i=7; i>=0; --i)
    //         std::cout << BitwiseOperations<unsigned char>::getBit(byte, i);

    //     // 清除第4位
    //     byte = BitwiseOperations<unsigned char>::clearBit(byte, 3);
        
    //     std::cout << "\nByte after clearing bit 4: ";
    //     for(int i=7; i>=0; --i)
    //         std::cout << BitwiseOperations<unsigned char>::getBit(byte, i);

    //     // 反转第5位
    //     byte = BitwiseOperations<unsigned char>::toggleBit(byte, 4);
        
    //     std::cout << "\nByte after toggling bit 5: ";
    //     for(int i=7; i>=0; --i)
    //         std::cout << BitwiseOperations<unsigned char>::getBit(byte, i);

    //     return 0;
    // }

#endif
#ifndef CONFIG_H
#define CONFIG_H
#include <cstddef>
namespace BwtFS{
    const uint8_t VERSION = 0;
    const size_t KB = 1024;
    const size_t MB = 1024 * KB;
    const size_t GB = 1024 * MB;
    const size_t TB = 1024 * GB;
    const size_t PB = 1024 * TB;
    const size_t EB = 1024 * PB;
    const size_t ZB = 1024 * EB;

    const size_t UNIT = KB;

    namespace SIZE{
        const size_t __SIZE_OF_VERSION = sizeof(VERSION);
    };
    
    namespace Config{
        const int BLOCK_SIZE = 4096;
    }
}
#endif
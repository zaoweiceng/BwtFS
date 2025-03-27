#ifndef CONFIG_H
#define CONFIG_H
namespace BwtFS{
    const char VERSION = 0;

    namespace SIZE{
        const unsigned int __SIZE_OF_VERSION = sizeof(VERSION);
    };
    
    namespace Config{
        const int BLOCK_SIZE = 4096;
    }
}
#endif
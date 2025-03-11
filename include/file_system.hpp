#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP

#include <string>

namespace BwtFS {
    class FileSystem {
    public:
        bool create_file(const std::string& path);
        bool delete_file(const std::string& path);
    };
}

#endif
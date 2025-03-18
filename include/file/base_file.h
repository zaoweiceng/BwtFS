#ifndef BASE_FILE_H
#define BASE_FILE_H
namespace BwtFS::File {
    class BaseFile {
    public:
        BaseFile();
        BaseFile(const BaseFile& other);
        BaseFile& operator=(const BaseFile& other);
        BaseFile(BaseFile&& other);
        BaseFile& operator=(BaseFile&& other);
        virtual void read() = 0;
        virtual void write() = 0;
        virtual void clear() = 0;
        virtual void print() = 0;
        
        virtual ~BaseFile();

    };
}
#endif
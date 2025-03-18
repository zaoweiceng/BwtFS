#ifndef BASE_NODE_H
#define BASE_NODE_H

namespace BwtFS::Node {

    class BaseNode {
    public:

        

        BaseNode();
        BaseNode(const BaseNode& other);
        BaseNode& operator=(const BaseNode& other);
        BaseNode(BaseNode&& other);
        BaseNode& operator=(BaseNode&& other);
        virtual void read() = 0;
        virtual void write() = 0;
        virtual void clear() = 0;
        virtual void print() = 0;
        virtual void setData(const char* data) = 0;
        virtual const char* getData() = 0;
        virtual ~BaseNode();

    };
}


#endif
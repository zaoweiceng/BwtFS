#ifndef DATE_H
#define DATE_H
#include <ctime>
#include <string>
namespace BwtFS::Util{
    std::string timeToString(const unsigned long long& time, const std::string& format = "%Y-%m-%d %H:%M:%S");
}
#endif
#include "util/date.h"

std::string BwtFS::Util::timeToString(const unsigned long long& time, const std::string& format){
    std::time_t t = static_cast<std::time_t>(time);
    std::tm tm = *std::localtime(&t);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), format.c_str(), &tm);
    return std::string(buffer);
}
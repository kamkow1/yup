#include <utils.h>

#include <sys/types.h>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <sstream>

std::string yupc::string_remove_all(std::string str, const std::string& from)
{
    size_t start_pos = 0;
    while(( start_pos = str.find( from)) != std::string::npos) {
        str.erase( start_pos, from.length());
    }

    return str;
}

u_int64_t yupc::string_to_uint64_t(std::string str)
{
    u_int64_t i;
    char *end;
    i = std::strtoull(str.c_str(), &end, 10);
    return i;
}

int64_t yupc::string_to_int64_t(std::string str)
{
    int64_t i;
    char *end;
    i = std::strtol(str.c_str(), &end, 10);
    return i;
}

std::vector<std::string> yupc::split_string(std::string &str, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(str);
    std::string e;

    while(std::getline(ss, e, delim)) {
        elems.push_back(e);
    }

    return elems;
}

std::string yupc::string_replace_all(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}
#include <utils.h>

#include <sys/types.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <sstream>

#include "llvm/Support/raw_ostream.h"

std::string yupc::StringRemoveAll(std::string str, const std::string& from)
{
    size_t startPos = 0;
    while(( startPos = str.find( from)) != std::string::npos) {
        str.erase( startPos, from.length());
    }

    return str;
}

u_int64_t yupc::StringToUInt64(std::string str)
{
    u_int64_t i;
    char *end;
    i = std::strtoull(str.c_str(), &end, 10);
    return i;
}

int64_t yupc::StringToInt64(std::string str)
{
    int64_t i;
    char *end;
    i = std::strtol(str.c_str(), &end, 10);
    return i;
}

std::vector<std::string> yupc::SplitString(std::string &str, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(str);
    std::string e;

    while(std::getline(ss, e, delim)) {
        elems.push_back(e);
    }

    return elems;
}

std::string yupc::StringReplaceAll(std::string str, const std::string &from, const std::string &to)
{
    size_t startPos = 0;
    while((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }

    return str;
}

void yupc::PrintLLVMType(llvm::Type *type)
{
    std::string str;
    llvm::raw_string_ostream rso(str);
    type->print(rso);

    std::cout << str << "\n";
}

void yupc::StringStripQuotes(std::string &str)
{
    str.erase(0, 1);
    str.erase(str.size() - 1);
}

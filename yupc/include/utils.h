#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>

#include "llvm/IR/Type.h"

namespace yupc
{
    std::string StringRemoveAll(std::string str, const std::string& from);

    u_int64_t StringToUInt64(std::string str);

    int64_t StringToInt64(std::string str);

    std::vector<std::string> SplitString(std::string &str, char delim);

    std::string StringReplaceAll(std::string str, const std::string &from, const std::string &to);

    void PrintLLVMType(llvm::Type *type);
}
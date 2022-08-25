#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>

#include <llvm/IR/Type.h>

namespace yupc
{
    std::string string_remove_all(std::string str, const std::string& from);

    u_int64_t string_to_uint64_t(std::string str);

    int64_t string_to_int64_t(std::string str);

    std::vector<std::string> split_string(std::string &str, char delim);

    std::string string_replace_all(std::string str, const std::string &from, const std::string &to);

    void dump_type_to_stdout(llvm::Type *type);
}
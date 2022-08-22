#include <llvm/IR/Module.h>

#include <string>

namespace yupc
{
    extern std::string build_dir;

    std::string init_bin_dir();

    std::string init_build_dir(std::string dir_base);

    void dump_module(llvm::Module *module, std::string module_name);
}
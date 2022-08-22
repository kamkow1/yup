#include <llvm/IR/Module.h>

#include <string>

namespace yupc
{
    extern std::string build_dir;

    std::string init_bin_dir();

    std::string init_build_dir(std::string dir_base);

    void dump_module(llvm::Module *module, std::string module_name);

    std::string file_to_str(const std::string& path);

    std::string get_dir_name(const std::string& fname);

    std::string get_ir_fname(std::string path);

    std::string base_name(std::string const &path);
}
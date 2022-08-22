#include <compiler/file_sys.h>
#include <compiler/compiler.h>
#include <msg/info.h>

#include <llvm/Support//FileSystem.h>

#include <filesystem>

namespace fs = std::filesystem;

std::string yupc::build_dir;

std::string yupc::init_build_dir(std::string dir_base) 
{

    fs::path b(dir_base);
    fs::path bd("build");
    fs::path f_path = b / bd;

    if (!fs::is_directory(f_path.string()) || !fs::exists(f_path.string())) 
    {
        fs::create_directory(f_path.string());
    }

    return f_path.string();
}

std::string yupc::init_bin_dir() 
{
    fs::path bin(yupc::build_dir);
    fs::path dir("bin");

    fs::path bin_dir = bin / dir;
    fs::create_directory(bin_dir.string());

    fs::path p(yupc::compiler_opts.binary_name + ".bc");
    fs::path bc_file = bin_dir / p;

    return bc_file;
}

void yupc::dump_module(llvm::Module *module, std::string module_name) 
{
    std::error_code ec;
    llvm::raw_fd_ostream os(module_name, ec, llvm::sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    std::string info = "module " + yupc::comp_units.back()->module_name;
    yupc::log_cmd_info(info);
}

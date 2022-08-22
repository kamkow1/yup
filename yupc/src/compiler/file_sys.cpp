#include <compiler/file_sys.h>
#include <compiler/compiler.h>
#include <msg/info.h>
#include <msg/errors.h>

#include <llvm/Support//FileSystem.h>

#include <filesystem>
#include <fstream>

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

std::string yupc::file_to_str(const std::string& path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        log_input_err("unable to open file \"" + path + "\"");
        exit(1);
    }

    auto beg = std::istreambuf_iterator<char>(input_file);
    auto end = std::istreambuf_iterator<char>();
    return std::string(beg, end);
}

std::string yupc::get_dir_name(const std::string& fname)
{
    auto pos = fname.find_last_of("\\/");
    return std::string::npos == pos ? "" : fname.substr(0, pos);
}

std::string yupc::get_ir_fname(std::string path)
{
    std::string base = path.substr(path.find_last_of("/\\") + 1);
    std::string directory = get_dir_name(path);

    std::string::size_type const p(base.find_last_of('.'));
    auto no_ext = base.substr(0, p);

    std::string full_name = directory + "/" + no_ext + ".ll";
    return full_name;
}

std::string yupc::base_name(std::string const &path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

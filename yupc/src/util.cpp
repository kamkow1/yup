#include <compiler/visitor.h>
#include <msg/errors.h>

#include <util.h>

#include <llvm/IR/Type.h>

#include <map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace yupc;
using namespace yupc::msg::errors;


namespace yu = yupc::util;

std::string yu::file_to_str(const std::string& path)
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

std::string yu::get_dir_name(const std::string& fname)
{
    auto pos = fname.find_last_of("\\/");
    return std::string::npos == pos ? "" : fname.substr(0, pos);
}

std::string yu::get_ir_fname(std::string path)
{
    auto base = path.substr(path.find_last_of("/\\") + 1);
    auto directory = get_dir_name(path);

    std::string::size_type const p(base.find_last_of('.'));
    auto no_ext = base.substr(0, p);

    auto full_name = directory + "/" + no_ext + ".ll";
    return full_name;
}

std::string yu::base_name(std::string const &path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

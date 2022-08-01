#include "compiler/visitor.h"
#include "messaging/errors.h"

#include "util.h"

#include "llvm/IR/Type.h"

#include "map"
#include "filesystem"
#include "iostream"
#include "fstream"
#include "filesystem"



std::string file_to_str(const std::string& path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        std::string errorMessage = "unable to open file \"" + path + "\"";
        log_input_err(errorMessage);
        exit(1);
    }

    std::istreambuf_iterator<char, std::char_traits<char>> beg 
        = std::istreambuf_iterator<char>(input_file);
    std::istreambuf_iterator<char, std::char_traits<char>> end 
        = std::istreambuf_iterator<char>();
    return std::string(beg, end);
}

std::string get_dir_name(const std::string& fname)
{
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos)
           ? ""
           : fname.substr(0, pos);
}

std::string get_ir_fname(std::string path)
{
    namespace fs = std::filesystem;

    std::string base = path.substr(path.find_last_of("/\\") + 1);
    std::string directory = get_dir_name(path);

    std::string::size_type const p(base.find_last_of('.'));
    std::string noExt = base.substr(0, p);

    fs::path fullName = directory + "/" + noExt + ".ll";
    return fullName;
}

#include <compiler/config.h>

#include <map>
#include <string>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::map<std::string, std::string> yupc::path_vars
{
    { "@std",  std::string(std::getenv("HOME")) + "/yup_stdlib" },
    { "@root", fs::current_path() }
};

#include "Compiler/Configuration.h"

#include <map>
#include <string>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::map<std::string, std::string> yupc::GlobalPathVariables
{
    { "@std",  std::string(std::getenv("HOME")) + "/yup_stdlib" },
    { "@root", fs::current_path() }
};

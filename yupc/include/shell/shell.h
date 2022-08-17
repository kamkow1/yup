#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace yupc::shell {

    void main_shell_loop(std::filesystem::path p);
}

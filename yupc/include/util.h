#pragma once

#include <string.h>

namespace yupc::util {
    std::string file_to_str(const std::string& path);

    std::string get_dir_name(const std::string& fname);

    std::string get_ir_fname(std::string path);

    std::string base_name(std::string const &path);
}

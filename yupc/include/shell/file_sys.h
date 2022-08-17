#pragma once

#include <fstream>
#include <string>

namespace yupc::shell::file_sys {

    extern std::ofstream fp;
    extern std::string fp_path;

    void close_dump_file(std::ofstream &fp);
}

#include <shell/file_sys.h>

#include <fstream>
#include <string>

using namespace yupc;

namespace shfs = shell::file_sys;

std::string shfs::fp_path;
std::ofstream shfs::fp;

void shfs::close_dump_file(std::ofstream &fp) {
    fp.close();
    remove(shfs::fp_path.c_str());
}

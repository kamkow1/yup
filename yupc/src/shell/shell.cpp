#include <compiler/compiler.h>
#include <shell/shell.h>
#include <shell/cmds.h>
#include <shell/file_sys.h>

#include <util.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <ios>
#include <map>
#include <filesystem>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

using namespace yupc;
using namespace boost;

namespace fs = std::filesystem;
namespace sc = shell::cmds;
namespace shfs = shell::file_sys;

void shell::main_shell_loop(fs::path p) {

    shfs::fp_path = p.string();
    shfs::fp.open(p.string(), std::ios::out);

    std::string input;

    std::cout << "starting yup shell...\n";
    std::cout << "yupc > ";
    while (std::getline(std::cin, input)) {
        std::cout << "yupc > ";
        
        std::string tmp  = input;
        std::vector<std::string> tokens;
        boost::split(tokens, input, algorithm::is_any_of("\t"));
        if (sc::shell_cmds.contains(tokens[0])) {
            sc::invoke_shell_cmd(sc::shell_cmds[input]);
            continue;
        }

        auto to_send = input + "\n";
        shfs::fp << to_send;
        shfs::fp.flush();
    }
}

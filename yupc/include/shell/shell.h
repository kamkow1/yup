#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace yupc::shell {

    enum ShellCmdTypes {
        SHELL_END,
        SHELL_COMPILE,
        SHELL_READ_BUF
    };

    extern std::map<std::string, ShellCmdTypes> shell_cmds;

    void main_shell_loop(std::filesystem::path p);

    void invoke_shell_cmd(ShellCmdTypes cmd_type);

    void close_dump_file(std::ofstream &fp);
}

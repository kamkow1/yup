#pragma once

#include <string>
#include <map>
#include <vector>

#define SHELL_CMD_NONE_ARG "--"

namespace yupc::shell::cmds {
    enum ShellCmdTypes {
        SHELL_END,
        SHELL_COMPILE,
        SHELL_READ_BUF
    };

    extern std::map<std::string, ShellCmdTypes> shell_cmds;

    void invoke_shell_cmd(ShellCmdTypes cmd_type, std::vector<std::string> args);
}
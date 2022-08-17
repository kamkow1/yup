#pragma once

#include <string>
#include <map>

namespace yupc::shell::cmds {
    enum ShellCmdTypes {
        SHELL_END,
        SHELL_COMPILE,
        SHELL_READ_BUF
    };

    extern std::map<std::string, ShellCmdTypes> shell_cmds;

    void invoke_shell_cmd(ShellCmdTypes cmd_type);
}
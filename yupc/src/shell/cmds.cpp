#include <compiler/compiler.h>
#include <shell/cmds.h>
#include <shell/shell.h>
#include <shell/file_sys.h>
#include <util.h>

#include <iostream>

using namespace yupc;

namespace sc = shell::cmds;
namespace shfs = shell::file_sys;

std::map<std::string, sc::ShellCmdTypes> sc::shell_cmds {
    { "!shell:end", sc::ShellCmdTypes::SHELL_END },
    { "!shell:compile", sc::ShellCmdTypes::SHELL_COMPILE },
    { "!shell:readbuf", sc::ShellCmdTypes::SHELL_READ_BUF }
};

void sc::invoke_shell_cmd(sc::ShellCmdTypes cmd_type) {

    switch (cmd_type) {
        case sc::ShellCmdTypes::SHELL_END: {
            std::cout << "\nclosing yup shell\n";

            shfs::close_dump_file(shfs::fp);
            exit(0);
        }

        case sc::ShellCmdTypes::SHELL_COMPILE: {
            std::cout << "\ncompiling buffer\n";

            compiler::process_path(shfs::fp_path);

            auto bc_file = compiler::init_bin_dir(compiler::build_dir);
            compiler::build_bitcode(bc_file, compiler::build_dir);
        }

        case sc::ShellCmdTypes::SHELL_READ_BUF: {
            std::cout << "\ncurrent buffer:\n";
            auto buf = util::file_to_str(shfs::fp_path);
            std::cout << buf << "\n";
        }
    }
}
#include <compiler/compiler.h>
#include <shell/cmds.h>
#include <shell/shell.h>
#include <shell/file_sys.h>
#include <util.h>

#include <iostream>
#include <vector>

using namespace yupc;

namespace sc = shell::cmds;
namespace shfs = shell::file_sys;

std::map<std::string, sc::ShellCmdTypes> sc::shell_cmds {
    { "!shell:end", sc::ShellCmdTypes::SHELL_END },
    { "!shell:compile", sc::ShellCmdTypes::SHELL_COMPILE },
    { "!shell:readbuf", sc::ShellCmdTypes::SHELL_READ_BUF },
    { "!shell:extsrc", sc::ShellCmdTypes::SHELL_EXT_SRC }
};

static std::vector<std::string> srcs;

void sc::invoke_shell_cmd(sc::ShellCmdTypes cmd_type, std::vector<std::string> args) {

    switch (cmd_type) {
        case sc::ShellCmdTypes::SHELL_END: {
            std::cout << "\nclosing yup shell\n";

            shfs::close_dump_file(shfs::fp);
            exit(0);
        }

        case sc::ShellCmdTypes::SHELL_COMPILE: {
            auto cwd = fs::current_path();
            compiler::build_dir = compiler::init_build_dir(cwd.string());

            srcs.push_back(shfs::fp_path);

            for (auto &path : srcs) {

                if (fs::is_directory(path)) {
                    for (auto &entry : fs::directory_iterator(path)) {
                        if (!fs::is_directory(entry)) {
                            compiler::process_path(entry.path().string());
                        }
                    }

                } else {

                    compiler::process_path(path);
                }
            }

            auto bc_file = compiler::init_bin_dir(compiler::build_dir);
            compiler::build_bitcode(bc_file, compiler::build_dir);
        }

        case sc::ShellCmdTypes::SHELL_READ_BUF: {
            auto buf = util::file_to_str(shfs::fp_path);
            std::cout << buf << "\n";
        }

        case sc::ShellCmdTypes::SHELL_EXT_SRC: {
            for (auto &aa : args) {

                srcs.push_back(aa);
            }
        }
    }
}

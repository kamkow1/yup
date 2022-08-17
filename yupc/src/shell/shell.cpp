#include <compiler/compiler.h>
#include <shell/shell.h>

#include <util.h>

#include <string>
#include <ios>
#include <map>
#include <filesystem>
#include <cstdio>
#include <iostream>
#include <fstream>

using namespace yupc;

namespace fs = std::filesystem;

static std::ofstream fp;
static std::string fp_path;

std::map<std::string, shell::ShellCmdTypes> shell::shell_cmds {
    { "!shell:end", shell::ShellCmdTypes::SHELL_END },
    { "!shell:compile", shell::ShellCmdTypes::SHELL_COMPILE },
    { "!shell:readbuf", shell::ShellCmdTypes::SHELL_READ_BUF }
};

void shell::close_dump_file(std::ofstream &fp) {
    fp.close();
    remove(fp_path.c_str());
}

void shell::invoke_shell_cmd(shell::ShellCmdTypes cmd_type) {

    switch (cmd_type) {
        case shell::ShellCmdTypes::SHELL_END: {
            std::cout << "closing yup shell\n";

            shell::close_dump_file(fp);
            exit(0);
        }

        case shell::ShellCmdTypes::SHELL_COMPILE: {
            std::cout << "\ncompiling buffer\n";

            compiler::process_path(fp_path);

            auto bc_file = compiler::init_bin_dir(compiler::build_dir);
            compiler::build_bitcode(bc_file, compiler::build_dir);
        }

        case shell::ShellCmdTypes::SHELL_READ_BUF: {
            std::cout << "\ncurrent buffer:\n";
            auto buf = util::file_to_str(fp_path);
            std::cout << buf << "\n";
        }
    }
}

void shell::main_shell_loop(fs::path p) {

    fp_path = p.string();
    fp.open(p.string(), std::ios::out);

    std::string input;

    std::cout << "starting yup shell...\n";
    std::cout << "yupc > ";
    while (std::getline(std::cin, input)) {
        std::cout << "yupc > ";
        
        if (shell::shell_cmds.contains(input)) {
            shell::invoke_shell_cmd(shell::shell_cmds[input]);
            continue;
        }

        auto to_send = input + "\n";
        fp << to_send;
        fp.flush();
    }
}

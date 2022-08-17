#include <compiler/compiler.h>
#include <shell.h>

#include <util.h>

#include <string>
#include <ios>
#include <map>
#include <filesystem>
#include <cstdio>
#include <iostream>

using namespace yupc;

namespace fs = std::filesystem;

static FILE *fp;
static std::string fp_path;

std::map<std::string, shell::ShellCmdTypes> shell::shell_cmds {
    { "!shell:end", shell::ShellCmdTypes::SHELL_END }
};

void shell::close_dump_file(FILE &fp) {
    fclose(&fp);
}

void shell::invoke_shell_cmd(shell::ShellCmdTypes cmd_type) {

    switch (cmd_type) {
        case shell::ShellCmdTypes::SHELL_END: {
            std::cout << "closing yup shell\n";

            shell::close_dump_file(*fp);
            int cl_res = remove(fp_path.c_str());

            exit(cl_res);
        }
    }
}

void shell::main_shell_loop(fs::path p) {
    fp_path = p.string();
    fp = fopen(p.string().c_str(), "w+");

    //auto bc_file = compiler::init_bin_dir(compiler::build_dir);
    //compiler::build_bitcode(bc_file, compiler::build_dir);

    std::string input;
    std::cin >> std::skipws;

    while (input.find("\n") == std::string::npos) {
        std::cout << "yupc > ";
        std::cin >> input;

        if (shell::shell_cmds.contains(input)) {

            shell::invoke_shell_cmd(shell::shell_cmds[input]);
        }

        std::string to_send = input + "\n";
        fwrite(to_send.c_str(), 1, input.size(), fp);
    }
}

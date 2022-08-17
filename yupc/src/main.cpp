#include <bits/types/FILE.h>
#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/compiler.h>
#include <compiler/import.h>
#include <compiler/type.h>
#include <shell/shell.h>

#include <util.h>

#include <CLI/CLI.hpp>

#include <string>
#include <filesystem>

using namespace llvm;
using namespace CLI;
using namespace yupc;

namespace fs = std::filesystem;

void init_build_opts(App *build_cmd, compiler::CompilerOpts *compiler_opts) {
    build_cmd->add_option("-s,--sources", compiler_opts->src_path, ".yup source files");
    build_cmd->add_option("-b,--binary-name", compiler_opts->binary_name, "sets the output binary's name");
    build_cmd->add_flag("-n,--no-perm", compiler_opts->give_perms, "allows the compiler to give permissions to the binary file");
    build_cmd->add_flag("-v,--verbose", compiler_opts->verbose, "enables verbose compiler output");
}

void init_shell_opts(App *shell_cmd, compiler::CompilerOpts *compiler_opts) {
    shell_cmd->add_option("-s,--sources", compiler_opts->src_path, ".yup source files in shell mode");
    shell_cmd->add_option("-b,--binary-name", compiler_opts->binary_name, "sets the output binary's name in shell mode");
    shell_cmd->add_flag("-n,--no-perm", compiler_opts->give_perms, "allows the compiler to give permissions to the binary file in shell mode");
    shell_cmd->add_flag("-v,--verbose", compiler_opts->verbose, "enables verbose compiler output in shell mode");
}

void process_build_cmd() {
    auto cwd = fs::current_path();
    compiler::build_dir = compiler::init_build_dir(cwd.string());

    for (auto &path : compiler::compiler_opts.src_path) {

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

void process_shell_cmd() {
    auto cwd = fs::current_path();
    compiler::build_dir = compiler::init_build_dir(cwd.string());

    fs::path dump_file("shell_dump.yup");
    auto p = cwd / dump_file;

    shell::main_shell_loop(p);
}

void setup_cmds(App &cli) {
    App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");
    App *shell_cmd = cli.add_subcommand("shell", "runs the yup compiler in a shell mode");

    init_build_opts(build_cmd, &compiler::compiler_opts);
    init_shell_opts(shell_cmd, &compiler::compiler_opts);

    build_cmd->callback([&]() {
        process_build_cmd();
    });

    shell_cmd->callback([&]() {
        process_shell_cmd();
    });
}

int main(int argc, char *argv[]) {

    App cli{"a compiler for the yup programming language"};

    setup_cmds(cli);

    CLI11_PARSE(cli, argc, argv);

    return 0;
}

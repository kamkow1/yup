#include <bits/types/FILE.h>
#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/compiler.h>

#include <util.h>

#include <CLI/CLI.hpp>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void init_build_opts(CLI::App *build_cmd, yupc::CompilerOpts *compiler_opts) 
{
    build_cmd->add_option("-s,--sources", compiler_opts->src_path, ".yup source files");
    build_cmd->add_option("-b,--binary-name", compiler_opts->binary_name, "sets the output binary's name");
    build_cmd->add_flag("-n,--no-perm", compiler_opts->give_perms, "allows the compiler to give permissions to the binary file");
    build_cmd->add_flag("-v,--verbose", compiler_opts->verbose, "enables verbose compiler output");
}

void process_build_cmd() 
{
    fs::path cwd = fs::current_path();
    yupc::build_dir = yupc::init_build_dir(cwd.string());

    for (std::string &path : yupc::compiler_opts.src_path) 
    {
        if (fs::is_directory(path)) 
        {
            for (auto &entry : fs::directory_iterator(path)) 
            {
                if (!fs::is_directory(entry)) 
                {
                    yupc::process_path(entry.path().string());
                }
            }

        } 
        else 
        {

            yupc::process_path(path);
        }
    }

    std::string bc_file = yupc::init_bin_dir();
    yupc::build_bitcode(bc_file, yupc::build_dir);
}

void setup_cmds(CLI::App &cli) {
    CLI::App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");
    init_build_opts(build_cmd, &yupc::compiler_opts);

    build_cmd->callback([&]() 
    {
        process_build_cmd();
    });
}

int main(int argc, char *argv[]) 
{
    CLI::App cli{"a compiler for the yup programming language"};

    setup_cmds(cli);
    CLI11_PARSE(cli, argc, argv);

    return 0;
}

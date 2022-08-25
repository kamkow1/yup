#include <compiler/Visitor.h>
#include <compiler/CompilationUnit.h>
#include <compiler/Compiler.h>
#include <compiler/FileSystem.h>
#include <compiler/Configuration.h>

#include <CLI/CLI.hpp>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void InitializeBuildCmdOptions(CLI::App *buildCmd, yupc::CompilerOptions *compilerOptions) 
{
    buildCmd->add_option("-s,--sources",       compilerOptions->SourcePaths,     ".yup source files");
    buildCmd->add_option("-b,--binary-name",   compilerOptions->BinaryName,      "sets the output binary's name");
    buildCmd->add_flag("-n,--no-perm",         compilerOptions->GivePermissions, "allows the compiler to give permissions to the binary file");
    buildCmd->add_flag("-v,--verbose",         compilerOptions->VerboseOutput,   "enables verbose compiler output");
}

void ProcessBuildCmd() 
{
    yupc::GlobalBuildDirPath = yupc::InitializeBuildDir(yupc::GlobalPathVariables["@root"]);

    for (std::string &path : yupc::GlobalCompilerOptions.SourcePaths)
    {
        if (fs::is_directory(path)) 
        {
            for (auto &entry : fs::directory_iterator(path)) 
            {
                if (!fs::is_directory(entry)) 
                {
                    yupc::ProcessPath(entry.path().string());
                }
            }
        } 
        else 
        {
            yupc::ProcessPath(path); // fail
        }
    }

    std::string bc_file = yupc::InitializeBinDir();
    yupc::BuildBitcode(bc_file);
}

void SetupCLICommands(CLI::App &cli) {
    CLI::App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");
    InitializeBuildCmdOptions(build_cmd, &yupc::GlobalCompilerOptions);

    build_cmd->callback([&]() {ProcessBuildCmd();});
}

int main(int argc, char *argv[]) 
{
    CLI::App cli{"a compiler for the yup programming language"};
    cli.allow_windows_style_options(true);

    SetupCLICommands(cli);
    CLI11_PARSE(cli, argc, argv);

    return 0;
}

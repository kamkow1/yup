#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Compiler.h"
#include "Compiler/FileSystem.h"
#include "Compiler/Configuration.h"
#include "utils.h"

#include "CLI/CLI.hpp"
#include "nlohmann/json.hpp"

#include <string>
#include <filesystem>

void InitializeBuildCmdOptions(CLI::App *buildCmd, yupc::CompilerOptions *compilerOptions) 
{
    buildCmd->add_option("-s,--sources",       compilerOptions->SourcePaths,     ".yup source files");
    buildCmd->add_option("-b,--binary-name",   compilerOptions->BinaryName,      "sets the output binary's name");
    buildCmd->add_flag("-n,--no-perm",         compilerOptions->GivePermissions, "allows the compiler to give permissions to the binary file");
    buildCmd->add_flag("-v,--verbose",         compilerOptions->VerboseOutput,   "enables verbose compiler output");
}

void InitializeBFCCmdOptions(CLI::App *bfcCmd, yupc::CompilerOptions *compilerConfigFile)
{
    bfcCmd->add_option("-f,--file",            compilerConfigFile->CompilerConfigFilePath, "sets the path to the configuration file for the compiler");
}

void ProcessBuildCmd() 
{
    yupc::GlobalBuildDirPath = yupc::InitializeBuildDir(yupc::GlobalPathVariables["@root"]);
    yupc::BuildProgram(yupc::GlobalCompilerOptions);
}

void ProcessBFCCmd()
{
    std::string jsonFile = yupc::ReadFileToString(yupc::GlobalCompilerOptions.CompilerConfigFilePath);
    nlohmann::json jsonObjectConfig = nlohmann::json::parse(jsonFile);
    nlohmann::json configObject = jsonObjectConfig["config"];
    yupc::LoadJsonConfigFile(configObject);
    yupc::GlobalBuildDirPath = yupc::InitializeBuildDir(yupc::GlobalPathVariables["@root"]);
    yupc::BuildProgram(yupc::GlobalCompilerOptions);
}

void SetupCLICommands(CLI::App &cli) {
    CLI::App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");
    CLI::App *bfc_cmd   = cli.add_subcommand("bfc"  , "loads a config file and uses it to manage the compilation process");

    InitializeBuildCmdOptions(build_cmd, &yupc::GlobalCompilerOptions);
    InitializeBFCCmdOptions(bfc_cmd, &yupc::GlobalCompilerOptions);

    build_cmd->callback([&]() {ProcessBuildCmd();});
    bfc_cmd->callback([&]()   {ProcessBFCCmd();});
}

int main(int argc, char *argv[]) 
{
    CLI::App cli{"a compiler for the yup programming language"};
    cli.allow_windows_style_options(true);

    SetupCLICommands(cli);
    CLI11_PARSE(cli, argc, argv);

    return 0;
}

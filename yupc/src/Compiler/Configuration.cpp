#include "Compiler/Configuration.h"

#include "Compiler/Compiler.h"
#include "nlohmann/json.hpp"

#include <map>
#include <string>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

yupc::CompilerConfigFile yupc::GlobalCompilerConfigFile;

std::map<std::string, std::string> yupc::GlobalPathVariables
{
    { "@std",  std::string(std::getenv("HOME")) + "/yup_stdlib" },
    { "@root", fs::current_path() }
};

void yupc::InitializeLoadedConfig(yupc::CompilerOptions &compilerOptions, yupc::CompilerConfigFile &configFile)
{
    compilerOptions.BinaryName    = configFile.binaryName;
    compilerOptions.VerboseOutput = configFile.verboseOutput;
    compilerOptions.SourcePaths.push_back(configFile.entryPoint);
}

void yupc::LoadJsonConfigFile(nlohmann::json &json)
{
    std::string entryPoint = json["main"].get<std::string>();
    std::string binaryName = json["bin"].get<std::string>();
    bool verboseOutput     = json["verbose"].get<bool>();

    yupc::GlobalCompilerConfigFile.entryPoint    = entryPoint;
    yupc::GlobalCompilerConfigFile.binaryName    = binaryName;
    yupc::GlobalCompilerConfigFile.verboseOutput = verboseOutput;

    yupc::InitializeLoadedConfig(yupc::GlobalCompilerOptions, yupc::GlobalCompilerConfigFile);
}


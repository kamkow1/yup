#ifndef YUPC_COMPILER_CONFIGURATION_H_
#define YUPC_COMPILER_CONFIGURATION_H_

#include "Compiler/Compiler.h"
#include "nlohmann/json.hpp"

#include <map>
#include <string>

namespace yupc
{
    struct CompilerConfigFile
    {
        std::string entryPoint;
        std::string binaryName;
        bool verboseOutput;
    };

    extern std::map<std::string, std::string> GlobalPathVariables;
    extern CompilerConfigFile GlobalCompilerConfigFile;

    void LoadJsonConfigFile(nlohmann::json &json);
    void InitializeLoadedConfig(yupc::CompilerOptions &compilerOptions, 
                                yupc::CompilerConfigFile &configFile);

} // namespace yupc

#endif

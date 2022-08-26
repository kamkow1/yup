#ifndef YUPC_COMPILER_COMPILER_H_
#define YUPC_COMPILER_COMPILER_H_

#include "Compiler/CompilationUnit.h"

#include "llvm/IR/Module.h"

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace yupc
{
    struct CompilerOptions
    {
        bool GivePermissions;
        bool VerboseOutput;
        std::vector<std::string> SourcePaths;  
        std::string BinaryName;
        std::string CompilerConfigFilePath;
    };

    extern CompilerOptions GlobalCompilerOptions;

    void BuildBitcode(fs::path binFile);
    void ProcessSourceFile(std::string path);
    void ProcessPath(std::string path);
    void BuildProgram(CompilerOptions &options);
    std::string CreateModuleName(std::string basePath);

} // namespace yupc

#endif

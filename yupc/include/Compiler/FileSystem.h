#ifndef YUPC_COMPILER_FILESYSTEM_H_
#define YUPC_COMPILER_FILESYSTEM_H_

#include "llvm/IR/Module.h"

#include <string>

namespace yupc
{
    extern std::string GlobalBuildDirPath;

    std::string InitializeBinDir();

    std::string InitializeBuildDir(std::string dirBase);

    void DumpModuleToIrFile(llvm::Module *module, std::string moduleName);

    std::string ReadFileToString(const std::string& path);

    std::string GetDirectoryName(const std::string& fname);

    std::string CreateIrFileName(std::string path);

    std::string GetBaseName(std::string const &path);
} // namespace yupc

#endif

#ifndef YUPC_COMPILER_IMPORT_H_
#define YUPC_COMPILER_IMPORT_H_

#include "Compiler/Type.h"

#include "llvm/IR/Module.h"

#include <string>
#include <map>
#include <vector>

namespace yupc 
{
    void ImportFunctions(llvm::Module &currentModule, llvm::Module &prevModule);

    void ImportGlobalVariables(llvm::Module &currentModule, llvm::Module &prevModule);

    void ImportTypeAliases(std::vector<yupc::AliasType*> &unit_alias_types, int i);

    void ImportPathRecursively(std::string path);

} // namespace yupc

#endif

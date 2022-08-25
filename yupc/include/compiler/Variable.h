#ifndef YUPC_COMPILER_VARIABLE_H_
#define YUPC_COMPILER_VARIABLE_H_

#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"

#include <string>
#include <cstddef>
#include <map>
#include <vector>

namespace yupc 
{

    struct VariableInfo
    {
        std::string Name;
        bool IsConstant;
        bool IsReference;
    };

    llvm::AllocaInst *FindLocalVariable(std::string name, size_t i,
        std::vector<std::map<std::string, llvm::AllocaInst*>> &symbolTable, std::string text);

    void IdentifierCodegen(std::string id, bool isGlobal);

    void AssignmentCodegen(std::string name, llvm::Value *val, std::string text);

    void VarDeclareCodegen(std::string name, llvm::Type *resolvedType, bool isConst, bool isGlobal, 
                            bool isPublic, bool isReference, llvm::Value *val = nullptr);

} // namespace yupc

#endif
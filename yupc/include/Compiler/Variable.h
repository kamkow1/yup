#ifndef YUPC_COMPILER_VARIABLE_H_
#define YUPC_COMPILER_VARIABLE_H_

#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"

#include <string>
#include <cstddef>
#include <map>
#include <vector>

namespace yupc 
{

    struct Variable
    {
        Variable(std::string  _name,
                bool          _isConstant,
                bool          _isReference,
                llvm::Value  *_valuePtr);

        std::string Name;
        bool IsConstant;
        bool IsReference;
        llvm::Value *ValuePtr;
    };

    yupc::Variable *FindLocalVariable(std::string name, size_t i,
        std::vector<std::map<std::string, yupc::Variable*>> &symbolTable, 
        size_t line, size_t pos, std::string text);

    llvm::LoadInst *IdentifierCodegen(std::string id, bool isGlobal, size_t line, 
                                        size_t pos, std::string text);
    void AssignmentCodegen(std::string name, llvm::Value *val, 
                    size_t line, size_t pos, std::string text);
    void VarDeclareCodegen(std::string name, llvm::Type *resolvedType, bool isConst, bool isGlobal, 
                            bool isPublic, bool isReference, llvm::Value *val = nullptr);

} // namespace yupc

#endif

#ifndef YUPC_COMPILER_FUNCITON_H_
#define YUPC_COMPILER_FUNCITON_H_

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"

#include <cstddef>
#include <vector>
#include <string>

namespace yupc 
{
    struct FunctionParameter
    {
    public:
        FunctionParameter();
        FunctionParameter(llvm::Type *_parameterType,
                        std::string _parameterName,
                        bool isVarArg);

        llvm::Type *ParameterType;
        std::string ParameterName;
        bool IsVarArg;
    };

    llvm::CallInst *FunctionCallCodegen(std::string functionName, std::vector<llvm::Value*> args, 
                            size_t line, size_t pos, std::string text);
    llvm::Value *FunctionReturnCodegen(llvm::Value *res, size_t line, size_t pos, std::string text);
    void FunctionDefinitionCodegen(llvm::Function *function);
    void FunctionSignatureCodegen(bool isVarArg, bool isPublic, std::string name, 
                    llvm::Type *return_type, std::vector<FunctionParameter*> params);

} // namespace yupc

#endif

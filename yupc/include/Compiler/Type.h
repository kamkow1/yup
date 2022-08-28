#ifndef YUPC_COMPILER_TYPE_H_
#define YUPC_COMPILER_TYPE_H_

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

#include <cstddef>
#include <map>
#include <vector>
#include <string>

namespace yupc 
{
    enum BuiltInTypes
    {
        LLVM_I32_TYPE,
        LLVM_I64_TYPE,
        LLVM_I8_TYPE,
        LLVM_I1_TYPE,
        LLVM_FLOAT_TYPE,
        LLVM_VOID_TYPE
    };

    struct AliasType 
    {
    public:
        AliasType();
        AliasType(std::string _typeName,
                std::string _destination,
                bool _isPublic);


        std::string TypeName;
        std::string Destination;
        bool IsPublic;
    };

    extern std::map<std::string, BuiltInTypes> BuiltInLLVMTypes;

    llvm::Type *ResolvePointerType(llvm::Type *base);
    
    llvm::Type *ResolveType(std::string typeName, llvm::LLVMContext &contextRef);

    llvm::Type *ResolveFixedArrayType(llvm::Type *base, u_int64_t size);

    llvm::Type *GetBuiltInLLVMType(std::string typeName, llvm::LLVMContext &contextRef);

    bool CheckValueType(llvm::Value *val1, llvm::Value *val2);

    std::string TypeToString(llvm::Type *type);
} // namespace yup

#endif
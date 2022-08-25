#ifndef YUPC_COMPILER_OPERATOR_H_
#define YUPC_COMPILER_OPERATOR_H_

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

#include <string>
#include <map>
#include <functional>

namespace yupc 
{

    struct OperatorArgument
    {
    public:
        OperatorArgument(llvm::Value *_operatorPossibleValue,
                        llvm::Type *_operatorPossibleType);

        llvm::Value *OperatorPossibleValue;
        llvm::Type *OperatorPossibleType;
    };

    extern std::map<std::string, std::function<llvm::Value*(std::vector<OperatorArgument*>&)>> ValueOperators;
    extern std::map<std::string, std::function<llvm::Type*(std::vector<OperatorArgument*>&)>>  TypeOperators;
} // namespace yupc

#endif

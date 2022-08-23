#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <string>
#include <map>
#include <functional>

namespace yupc 
{

    union OperArg 
    {
        llvm::Value *oper_value;
        llvm::Type *oper_type;
    };

    extern std::map<std::string, std::function<llvm::Value*(std::vector<yupc::OperArg*>&)>> value_operators;
    extern std::map<std::string, std::function<llvm::Type*(std::vector<yupc::OperArg*>&)>> type_operators;
}

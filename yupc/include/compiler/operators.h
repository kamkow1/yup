#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <string>
#include <map>
#include <stdarg.h>

namespace yupc::compiler::operators {

    union OperArg {
        llvm::Value *oper_value;
        llvm::Type *oper_type;
    };

    extern std::map<std::string, llvm::Value *(*)(std::vector<OperArg*>)> opers;
}

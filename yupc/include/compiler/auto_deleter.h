#include <llvm/IR/Instructions.h>

#include <string>
#include <vector>


namespace yupc::compiler::auto_deleter {

    void mark_variable(std::string name);

    void free_call_codegen(int i);

    extern std::vector<llvm::AllocaInst*> marked_vars;
}

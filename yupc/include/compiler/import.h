#pragma once

#include <compiler/type.h>

#include <llvm/IR/Module.h>

#include <string>
#include <map>

namespace yupc 
{
    struct ImportDecl 
    {
        std::vector<std::string> sym_names;
        std::string mod_name;
    };

    void import_funcs(llvm::Module &current_mod, llvm::Module &prev_mod);

    void import_global_var(llvm::Module &current_mod, llvm::Module &prev_mod);

    void import_type_alias(std::vector<yupc::AliasType*> &unit_alias_types, int i);
}

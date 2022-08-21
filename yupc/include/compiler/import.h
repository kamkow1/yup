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

    void import_funcs(llvm::Module &current_mod, llvm::Module &prev_mod, std::string text);

    void import_global_var(std::map<std::string, llvm::GlobalVariable*> global_vars, std::string sym, std::string text);

    void import_type_alias(std::vector<yupc::AliasType*> &unit_alias_types, int i);
}

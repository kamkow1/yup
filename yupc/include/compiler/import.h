#pragma once

#include <compiler/type.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>

#include <string>
#include <map>

namespace yupc::compiler::import {
    struct ImportDecl {
        std::vector<std::string> sym_names;
        std::string mod_name;
    };

    void import_func(std::map<std::string, llvm::Function*> &funcs, std::string sym);

    void import_global_var(std::map<std::string, llvm::GlobalVariable*> global_vars, std::string sym);

    void import_type_alias(std::vector<yupc::compiler::type::AliasType*> &unit_alias_types, int i);
}

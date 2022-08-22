#include <compiler/compiler.h>
#include <compiler/visitor.h>
#include <compiler/import.h>
#include <compiler/compilation_unit.h>
#include <compiler/type.h>
#include <llvm/Support/Casting.h>
#include <msg/errors.h>
#include <tree/TerminalNode.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>


#include <cstddef>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

void yupc::import_funcs(llvm::Module &current_mod, llvm::Module &prev_mod) 
{

    for (llvm::Function &func : current_mod.functions()) {

        if (func.isPrivateLinkage(func.getLinkage())) {
            continue;
        }

        std::string return_type_str;
        llvm::raw_string_ostream return_type_rso(return_type_str);

        func.getReturnType()->print(return_type_rso);
        llvm::Type *return_type = yupc::resolve_type(return_type_rso.str(), prev_mod.getContext());

        std::vector<llvm::Type*> param_types;
        for (size_t i = 0; i < func.arg_size(); i++) 
        {
            std::string type_name;
            llvm::raw_string_ostream param_type_rso(type_name);
            
            func.getFunctionType()->getParamType(i)->print(param_type_rso);

            llvm::Type *param_type = yupc::resolve_type(param_type_rso.str(), prev_mod.getContext());

            param_types.push_back(param_type);
        }

        llvm::FunctionType *func_type = llvm::FunctionType::get(return_type, param_types, false);
        llvm::Function::LinkageTypes linkage_type = llvm::Function::ExternalLinkage;
        llvm::FunctionCallee fn_callee = prev_mod.getOrInsertFunction(func.getName(), func_type);
        llvm::Function *created_func = llvm::cast<llvm::Function*>(fn_callee);

        yupc::comp_units[yupc::comp_units.size() - 2]->functions[func.getName().str()] = created_func;
    }
}

void yupc::import_global_var(llvm::Module &current_mod, llvm::Module &prev_mod) 
{
    for (llvm::GlobalVariable &gvar : current_mod.globals())
    {
        if (gvar.isPrivateLinkage(gvar.getLinkage()))
        {
            continue;
        }

        std::string gvar_type_str;  
        llvm::raw_string_ostream rso(gvar_type_str);
        gvar.getValueType()->print(rso);
        auto *gvar_type = yupc::resolve_type(rso.str(), prev_mod.getContext());

        bool is_const = gvar.isConstant();

        llvm::GlobalValue::LinkageTypes linkage_type = llvm::GlobalValue::ExternalLinkage;
        llvm::GlobalVariable *global_var = new llvm::GlobalVariable(prev_mod, gvar_type, is_const, 
                                                        linkage_type, nullptr, gvar.getName());

        yupc::comp_units[yupc::comp_units.size() - 2]->global_variables[gvar.getName().str()] = global_var;
    }
}

void yupc::import_type_alias(std::vector<yupc::AliasType*> &unit_alias_types, int i) {

    yupc::AliasType *alias_type = unit_alias_types[i];

    yupc::comp_units.back()->alias_types.push_back(alias_type);
}

void yupc::import_path_recursive(std::string path)
{
    if (fs::is_directory(path))
    {
        for (auto &entry : fs::directory_iterator(path))
        {
            yupc::import_path_recursive(entry.path().string());
        }
    }
    else
    {
        yupc::process_path(path);

        yupc::import_funcs(*yupc::comp_units.back()->module, 
                    *yupc::comp_units[yupc::comp_units.size() - 2]->module);

        yupc::import_global_var(*yupc::comp_units.back()->module, 
                        *yupc::comp_units[yupc::comp_units.size() - 2]->module);

        yupc::comp_units.pop_back();
    }
}

std::any yupc::Visitor::visitImport_decl(yupc::YupParser::Import_declContext *ctx) {
    
    std::string module_name = ctx->V_STRING()->getText();
    module_name = module_name.substr(1, module_name.size() - 2);

    yupc::import_path_recursive(module_name);

    return nullptr;
}

#include <compiler/visitor.h>
#include <compiler/import.h>
#include <compiler/compilation_unit.h>
#include <compiler/module.h>
#include <compiler/type.h>
#include <msg/errors.h>
#include <tree/TerminalNode.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>


#include <cstddef>
#include <string>
#include <vector>

void yupc::import_func(std::map<std::string, llvm::Function*> &funcs, std::string sym, std::string text) 
{
    if (!funcs.contains(sym)) 
    {
        yupc::log_compiler_err("cannot import function \"" + sym + "\" because it doesn't exist", text);
        exit(1);
    }

    std::string return_type_str;
    llvm::raw_string_ostream return_type_rso(return_type_str);

    funcs[sym]->getFunctionType()->getReturnType()->print(return_type_rso);

    llvm::Type *return_type = yupc::resolve_type(return_type_rso.str());

    std::vector<llvm::Type*> param_types;
    for (size_t i = 0; i < funcs[sym]->arg_size(); i++) 
    {
        std::string type_name;
        llvm::raw_string_ostream param_type_rso(type_name);
        
        funcs[sym]->getFunctionType()->getParamType(i)->print(param_type_rso);

        auto *param_type = yupc::resolve_type(param_type_rso.str());

        param_types.push_back(param_type);
    }

    llvm::FunctionType *func_type = llvm::FunctionType::get(return_type, param_types, false);
    llvm::Function::LinkageTypes linkage_type = llvm::Function::ExternalLinkage;
    llvm::Function *func = llvm::Function::Create(func_type, linkage_type, sym, yupc::comp_units.back()->module);

    for (size_t i = 0; i < func->arg_size(); i++) 
    {
        func->addParamAttr(i, llvm::Attribute::NoUndef);
    }     

    yupc::comp_units.back()->functions[sym] = func;
}

void yupc::import_global_var(std::map<std::string, llvm::GlobalVariable*> global_vars, std::string sym, std::string text) 
{
    if (!global_vars.contains(sym)) 
    {
        yupc::log_compiler_err("cannot import global variable \"" + sym + "\" because it doesn't exist", text);
        exit(1);
    }

    std::string gvar_type_str;
    llvm::raw_string_ostream rso(gvar_type_str);
    global_vars[sym]->getType()->print(rso);
    auto *gvar_type = yupc::resolve_type(rso.str());

    bool is_const = global_vars[sym]->isConstant();

    llvm::GlobalValue::LinkageTypes linkage_type = llvm::GlobalValue::ExternalLinkage;
    llvm::GlobalVariable *global_var = new llvm::GlobalVariable(*yupc::comp_units.back()->module, gvar_type, is_const, linkage_type, nullptr, sym);

    yupc::comp_units.back()->global_variables[sym] = global_var;
}

void yupc::import_type_alias(std::vector<yupc::AliasType*> &unit_alias_types, int i) {

    yupc::AliasType *alias_type = unit_alias_types[i];

    yupc::comp_units.back()->alias_types.push_back(alias_type);
}

std::any yupc::Visitor::visitImport_decl(yupc::YupParser::Import_declContext *ctx) {
    
    std::string module_name = ctx->IDENTIFIER()->getText();

    yupc::ImportDecl import_decl {
        std::vector<std::string>(),
        module_name
    };        

    for (antlr4::tree::TerminalNode *import_item : ctx->import_list()->IDENTIFIER()) {
        import_decl.sym_names.push_back(import_item->getText());
    }

    for (std::string &sym : import_decl.sym_names) {

        for (yupc::CompilationUnit *unit : yupc::comp_units) {

            if (unit->module_id == import_decl.mod_name) {

                for (auto &func : unit->functions) {
                    if (func.first == sym) {
                        yupc::import_func(unit->functions, sym, ctx->getText());
                    }
                }

                for (auto &global_var : unit->global_variables) {
                    if (global_var.first == sym) {
                        yupc::import_global_var(unit->global_variables, sym, ctx->getText());
                    }
                }

                int i = 0;
                for (yupc::AliasType *type_alias : unit->alias_types) {
                    if (type_alias->type_name == sym) {
                        yupc::import_type_alias(unit->alias_types, i);
                    }

                    i++;
                }
            }
        }
    }

    return nullptr;
}

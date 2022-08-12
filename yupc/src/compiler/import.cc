#include <compiler/visitor.h>
#include <compiler/import.h>
#include <compiler/compilation_unit.h>
#include <compiler/module.h>
#include <compiler/type.h>

#include <msg/errors.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>

#include <string>
#include <vector>

using namespace yupc;
using namespace llvm;

namespace cv = compiler::visitor;
namespace com_un = compiler::compilation_unit;
namespace ci = compiler::import;
namespace ct = compiler::type;

void ci::import_func(std::map<std::string, Function*> &funcs, std::string sym) {
    if (!funcs.contains(sym)) {
        msg::errors::log_compiler_err("cannot import function \"" + sym + "\" because it doesn't exist");
        exit(1);
    }

    std::string return_type_str;
    raw_string_ostream return_type_rso(return_type_str);

    funcs[sym]->getFunctionType()->getReturnType()->print(return_type_rso);

    auto *return_type = ct::resolve_type(return_type_rso.str());

    std::vector<Type*> param_types;
    for (auto i = 0; i < funcs[sym]->arg_size(); i++) {

        std::string type_name;
        raw_string_ostream param_type_rso(type_name);
        
        funcs[sym]->getFunctionType()->getParamType(i)->print(param_type_rso);

        auto *param_type = ct::resolve_type(param_type_rso.str());

        param_types.push_back(param_type);
    }

    auto *func_type = FunctionType::get(return_type, param_types, false);
    auto *func = Function::Create(func_type, Function::ExternalLinkage, sym, com_un::comp_units.back()->module);

    for (auto i = 0; i < func->arg_size(); i++) {
        func->addParamAttr(i, Attribute::NoUndef);
    }     

    com_un::comp_units.back()->functions[sym] = func;
}

void ci::import_global_var(std::map<std::string, GlobalVariable*> global_vars, std::string sym) {
    if (!global_vars.contains(sym)) {
        msg::errors::log_compiler_err("cannot import global variable \"" + sym + "\" because it doesn't exist");
        exit(1);
    }

    auto *gvar_type = global_vars[sym]->getType();
    auto is_const = global_vars[sym]->isConstant();

    GlobalVariable *global_var = new GlobalVariable(gvar_type, is_const, GlobalValue::LinkageTypes::ExternalLinkage);

    com_un::comp_units.back()->global_variables[sym] = global_var;
}

std::any cv::Visitor::visitImport_decl(parser::YupParser::Import_declContext *ctx) {
    
    auto module_name = ctx->IDENTIFIER()->getText();

    ci::ImportDecl import_decl {
        std::vector<std::string>{},
        module_name
    };        

    for (auto *import_item : ctx->import_list()->IDENTIFIER()) {
        import_decl.sym_names.push_back(import_item->getText());
    }

    for (auto &sym : import_decl.sym_names) {

        for (auto *unit : com_un::comp_units) {

            if (unit->module_id == import_decl.mod_name) {

                for (auto &func : unit->functions) {
                    if (func.first == sym) {
                        ci::import_func(unit->functions, sym);
                    }
                }

                for (auto &global_var : unit->global_variables) {
                    if (global_var.first == sym) {
                        ci::import_global_var(unit->global_variables, sym);
                    }
                }
            }
        }
    }

    return nullptr;
}

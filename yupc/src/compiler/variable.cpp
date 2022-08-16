#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/variable.h>
#include <compiler/compilation_unit.h>
#include <compiler/auto_deleter.h>

#include <msg/errors.h>
#include <util.h>

#include <llvm/Support/TypeName.h>
#include <llvm/Support/Alignment.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Metadata.h>

#include <cstdlib>

using namespace llvm;
using namespace yupc;
using namespace yupc::msg::errors;

namespace cv = compiler::visitor;
namespace cvar = compiler::variable;
namespace ct = compiler::type;
namespace com_un = compiler::compilation_unit;

struct Variable {
    std::string name;
    bool is_const;
};

static std::map<std::string, Variable> variables;

void cvar::ident_expr_codegen(std::string id, bool is_glob) {

    auto contains_id = com_un::comp_units.back()->symbol_table.back().contains(id);

    if (contains_id) {

        auto *value =com_un::comp_units.back()->symbol_table.back()[id];

        auto *type = value->getAllocatedType();

        auto *load = com_un::comp_units.back()->ir_builder->CreateLoad(type, value);

        com_un::comp_units.back()->value_stack.push(load);
    } else if (is_glob) {
        
        auto *gv = com_un::comp_units.back()->global_variables[id];

        auto *load = com_un::comp_units.back()->ir_builder->CreateLoad(gv->getValueType(), gv);

        com_un::comp_units.back()->value_stack.push(load);

    } else {

        auto *gv = com_un::comp_units.back()->global_variables[id];

        auto *type = gv->getValueType();

        auto *load = com_un::comp_units.back()->ir_builder->CreateLoad(type, gv);

        com_un::comp_units.back()->value_stack.push(load);
    }

}

void cvar::assignment_codegen(std::string name, Value *val) {

    auto var = variables[name];

    if (var.is_const) {
        log_compiler_err("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    auto is_local = com_un::comp_units.back()->symbol_table.back().contains(name);
    auto is_global = com_un::comp_units.back()->global_variables.contains(name);

    if (is_local) {
        auto *stored = com_un::comp_units.back()->symbol_table.back()[name];

        ct::check_value_type(val, name);

        com_un::comp_units.back()->ir_builder->CreateStore(val, stored, false);

        com_un::comp_units.back()->value_stack.pop();
    } else if (is_global) {
        auto *gv = com_un::comp_units.back()->global_variables[name];

        ct::check_value_type(val, name);

        com_un::comp_units.back()->ir_builder->CreateStore(val, gv, false);

        com_un::comp_units.back()->value_stack.pop();
    } else {
        log_compiler_err("cannot reassign \"" + name + "\" because it doesn't exist");
        exit(1);
    }
}

void cvar::var_declare_codegen(std::string name, Type *resolved_type, bool is_const, bool is_glob, bool is_ext, Value *val) {

    if (is_glob) {
        auto lt = is_ext ? GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;

        auto *gv = new GlobalVariable(*com_un::comp_units.back()->module, resolved_type, is_const, lt, 0);

        gv->setDSOLocal(true);

        gv->setInitializer((Constant*) val);

        com_un::comp_units.back()->global_variables[name] = gv;
        com_un::comp_units.back()->value_stack.push(gv);

        Variable var{name, is_const};
        variables[name] = var;
    } else {
        auto *ptr = com_un::comp_units.back()->ir_builder->CreateAlloca(resolved_type, 0, "");

        if (val != nullptr) {
            com_un::comp_units.back()->ir_builder->CreateStore(val, ptr, false);
        }

        com_un::comp_units.back()->symbol_table.back()[name] = ptr;

        com_un::comp_units.back()->value_stack.push(ptr);

        Variable var{name, is_const};
        variables[name] = var;

        //if (ptr->getAllocatedType()->isPointerTy()) {
        //    auto_del::mark_variable(name);
        //}
    }
}

std::any cv::Visitor::visitVar_declare(parser::YupParser::Var_declareContext *ctx) {

    auto name = ctx->IDENTIFIER()->getText();
    
    auto is_const = ctx->CONST() != nullptr;
    auto is_glob = ctx->GLOBAL() != nullptr;
    auto is_pub = ctx->PUBSYM() != nullptr;

    auto glob_contains = com_un::comp_units.back()->global_variables.contains(name);

    if (is_glob && glob_contains) {
        msg::errors::log_compiler_err("global variable \"" + name + ctx->type_annot()->getText() + "\" already exists");
        exit(1);
    }

    auto loc_constains = false;
    if (com_un::comp_units.back()->symbol_table.size() != 0) {
        com_un::comp_units.back()->symbol_table.back().contains(name);
    }

    if (!is_glob && loc_constains) {
        msg::errors::log_compiler_err("variable \"" + name + ctx->type_annot()->getText() + "\" has already been declared in this scope");
        exit(1);
    }

    auto *resolved_type = std::any_cast<Type*>(this->visit(ctx->type_annot()));

    Value *val = nullptr;
    if (ctx->var_value() != nullptr) {
        this->visit(ctx->var_value()->expr());
        val = com_un::comp_units.back()->value_stack.top();
    }

    cvar::var_declare_codegen(name, resolved_type, is_const, is_glob, is_pub, val);

    return nullptr;
}

std::any cv::Visitor::visitAssignment(parser::YupParser::AssignmentContext *ctx) {

    auto name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    auto *val = com_un::comp_units.back()->value_stack.top();

    cvar::assignment_codegen(name, val);
    
    return nullptr;
}

std::any cv::Visitor::visitIdentifierExpr(parser::YupParser::IdentifierExprContext *ctx) {

    auto name = ctx->IDENTIFIER()->getText();

    auto is_glob = com_un::comp_units.back()->global_variables.contains(name);

    auto is_loc = com_un::comp_units.back()->symbol_table.back().contains(name);

    if (!is_glob && !is_loc) {
        msg::errors::log_compiler_err("symbol \"" + name + "\" is neither a local nor a global variable");

        exit(1);
    }


    cvar::ident_expr_codegen(name, is_glob);
    
    return nullptr;
}
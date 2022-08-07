#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/variable.h>
#include <compiler/compilation_unit.h>
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
    bool        is_const;
};

static std::map<std::string, Variable> variables;

void cvar::ident_expr_codegen(std::string id) {

    bool contains_id = com_un::comp_units.top()->symbol_table.top().contains(id);

    if (contains_id) {
        AllocaInst *value =com_un::comp_units.top()->symbol_table.top()[id];

        Type *type = value->getAllocatedType();

        LoadInst *load = com_un::comp_units.top()->ir_builder.CreateLoad(type, value);

        com_un::comp_units.top()->value_stack.push(load);
    } else {
        GlobalVariable *gv = com_un::comp_units.top()->global_variables[id];

        Type *type = gv->getValueType();

        LoadInst *load = com_un::comp_units.top()->ir_builder.CreateLoad(type, gv);

        com_un::comp_units.top()->value_stack.push(load);
    }

}

void cvar::assignment_codegen(std::string name, Value *val) {

    Variable var = variables[name];

    if (var.is_const) {
        log_compiler_err("cannot reassign a constant \"" + name + "\"");
        exit(1);
    }

    bool contains_name = com_un::comp_units.top()->symbol_table.top().contains(name);

    if (contains_name) {
        AllocaInst *stored = com_un::comp_units.top()->symbol_table.top()[name];

        ct::check_value_type(val, name);

        com_un::comp_units.top()->ir_builder.CreateStore(val, stored, false);

        com_un::comp_units.top()->value_stack.pop();
    } else {
        GlobalVariable *gv = com_un::comp_units.top()->global_variables[name];

        ct::check_value_type(val, name);

        com_un::comp_units.top()->ir_builder.CreateStore(val, gv, false);

        com_un::comp_units.top()->value_stack.pop();
    }
}

void cvar::var_declare_codegen(std::string name, Type *resolved_type, 
                        bool is_const, bool is_glob, bool is_ext, Value *val) {

    if (is_glob) {
        GlobalValue::LinkageTypes lt = is_ext 
            ? GlobalValue::ExternalLinkage 
            : GlobalValue::InternalLinkage;

        GlobalVariable *gv = new GlobalVariable(*com_un::comp_units.top()->module, 
                                                resolved_type, 
                                                is_const, 
                                                lt, 0);


        gv->setDSOLocal(true);

        gv->setInitializer((Constant*) val);

        com_un::comp_units.top()->global_variables[name] = gv;

        com_un::comp_units.top()->value_stack.push(gv);

        Variable var{name, is_const};
        variables[name] = var;
    } else {
        AllocaInst *ptr = com_un::comp_units.top()->ir_builder.CreateAlloca(resolved_type, 0, "");

        if (val != nullptr) {
            com_un::comp_units.top()->ir_builder.CreateStore(val, ptr, false);
        }

        com_un::comp_units.top()->symbol_table.top()[name] = ptr;

        com_un::comp_units.top()->value_stack.push(ptr);

        Variable var{name, is_const};
        variables[name] = var;
    }
}

std::any cv::Visitor::visitVar_declare(parser::YupParser::Var_declareContext *ctx) {

    std::string name = ctx->IDENTIFIER()->getText();
    
    bool is_const = ctx->CONST() != nullptr;
    bool is_glob = ctx->GLOBAL() != nullptr;
    bool is_ext = ctx->EXPORT() != nullptr;

    bool glob_contains = com_un::comp_units.top()->global_variables.contains(name);

    if (is_glob && glob_contains) {
        msg::errors::log_compiler_err("global variable \"" + name 
            + ctx->type_annot()->getText() + "\" already exists");

        exit(1);
    }

    bool loc_constains = com_un::comp_units.top()->symbol_table.top().contains(name);

    if (!is_glob && loc_constains) {
        msg::errors::log_compiler_err("variable \"" + name 
            + ctx->type_annot()->getText()
            + "\" has already been declared in this scope");
        exit(1);
    }

    Type *resolved_type = std::any_cast<Type*>(this->visit(ctx->type_annot()));

    Value *val = nullptr;
    if (ctx->var_value() != nullptr) {
        this->visit(ctx->var_value()->expr());
        val = com_un::comp_units.top()->value_stack.top();
    }

    cvar::var_declare_codegen(name, 
                            resolved_type, 
                            is_const, 
                            is_glob, 
                            is_ext, 
                            val);

    return nullptr;
}

std::any cv::Visitor::visitAssignment(parser::YupParser::AssignmentContext *ctx) {

    std::string name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    Value *val = com_un::comp_units.top()->value_stack.top();

    cvar::assignment_codegen(name, val);
    
    return nullptr;
}

std::any cv::Visitor::visitIdentifierExpr(parser::YupParser::IdentifierExprContext *ctx) {

    std::string name = ctx->IDENTIFIER()->getText();

    bool is_glob = com_un::comp_units.top()->global_variables.contains(name);

    bool is_loc = com_un::comp_units.top()->symbol_table.top().contains(name);

    if (!is_glob || !is_loc) {
        msg::errors::log_compiler_err("symbol \"" 
            + name + "\" is neither a local nor a global variable");

        exit(1);
    }


    cvar::ident_expr_codegen(name);
    
    return nullptr;
}
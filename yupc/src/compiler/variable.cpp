#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/variable.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <msg/errors.h>
#include <string>
#include <util.h>

#include <llvm/Support/TypeName.h>
#include <llvm/Support/Alignment.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Metadata.h>

#include <cstdlib>

static std::map<std::string, yupc::Variable> variables;

void yupc::ident_expr_codegen(std::string id, bool is_glob) 
{

    bool contains_id = yupc::comp_units.back()->symbol_table.back().contains(id);

    if (contains_id) 
    {
        llvm::AllocaInst *value =yupc::comp_units.back()->symbol_table.back()[id];
        llvm::Type *type = value->getAllocatedType();

        llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(type, value);

        yupc::comp_units.back()->value_stack.push(load);
    } 
    else if (is_glob) 
    {
        llvm::GlobalVariable *gv = yupc::comp_units.back()->global_variables[id];
        llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(gv->getValueType(), gv);

        yupc::comp_units.back()->value_stack.push(load);
    } 
    else 
    {
        llvm::GlobalVariable *gv = yupc::comp_units.back()->global_variables[id];
        llvm::Type *type = gv->getValueType();

        llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(type, gv);

        yupc::comp_units.back()->value_stack.push(load);
    }

}

void yupc::assignment_codegen(std::string name, llvm::Value *val, std::string text) 
{

    auto var = variables[name];

    if (var.is_const) 
    {
        yupc::log_compiler_err("cannot reassign a constant \"" + name + "\"", text);
    }

    if (var.is_ref) 
    {
        yupc::log_compiler_err("cannot make a reference point to another variable", text);
    }

    bool is_local = yupc::comp_units.back()->symbol_table.back().contains(name);
    bool is_global = yupc::comp_units.back()->global_variables.contains(name);

    if (is_local) 
    {
        llvm::AllocaInst *stored = yupc::comp_units.back()->symbol_table.back()[name];
        yupc::check_value_type(val, name);

        yupc::comp_units.back()->ir_builder->CreateStore(val, stored, false);

        yupc::comp_units.back()->value_stack.pop();
    } 
    else if (is_global) 
    {
        llvm::GlobalVariable *gv = yupc::comp_units.back()->global_variables[name];

        yupc::check_value_type(val, name);

        yupc::comp_units.back()->ir_builder->CreateStore(val, gv, false);

        yupc::comp_units.back()->value_stack.pop();
    }
    else 
    {
        yupc::log_compiler_err("cannot reassign \"" + name + "\" because it doesn't exist", text);
    }
}

void yupc::var_declare_codegen(std::string name, llvm::Type *resolved_type, bool is_const, 
                            bool is_glob, bool is_ext, bool is_ref, llvm::Value *val) 
{

    if (is_glob) 
    {
        llvm::GlobalValue::LinkageTypes lt = is_ext ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::PrivateLinkage;

        llvm::GlobalVariable *gv = new llvm::GlobalVariable(*yupc::comp_units.back()->module, resolved_type, 
                                                        is_const, lt, 0, name);

        gv->setDSOLocal(true);

        gv->setInitializer((llvm::Constant*) val);

        yupc::comp_units.back()->global_variables[name] = gv;
        yupc::comp_units.back()->value_stack.push(gv);

        Variable var{name, is_const, is_ref};
        variables[name] = var;
    } 
    else 
    {
        llvm::AllocaInst *ptr = yupc::comp_units.back()->ir_builder->CreateAlloca(resolved_type, 0, "");

        if (val != nullptr) 
        {
            yupc::comp_units.back()->ir_builder->CreateStore(val, ptr, false);
        }

        yupc::comp_units.back()->symbol_table.back()[name] = ptr;
        yupc::comp_units.back()->value_stack.push(ptr);

        Variable var{name, is_const, is_ref};
        variables[name] = var;
    }
}

std::any yupc::Visitor::visitVar_declare(yupc::YupParser::Var_declareContext *ctx) 
{
    std::string name = ctx->IDENTIFIER()->getText();
    
    bool is_const = ctx->CONST() != nullptr;
    bool is_glob = ctx->GLOBAL() != nullptr;
    bool is_pub = ctx->PUBSYM() != nullptr;
    bool is_ref = ctx->REF() != nullptr;

    if (is_ref && ctx->var_value() == nullptr) 
    {
        yupc::log_compiler_err("cannot declare a reference that doesn't point to a variable", ctx->getText());
    }

    bool glob_contains = yupc::comp_units.back()->global_variables.contains(name);

    if (is_glob && glob_contains) 
    {
        yupc::log_compiler_err("global variable \"" + name + ctx->type_annot()->getText() + "\" already exists", ctx->getText());
    }

    bool loc_constains = false;
    if (yupc::comp_units.back()->symbol_table.size() != 0) 
    {
        yupc::comp_units.back()->symbol_table.back().contains(name);
    }

    if (!is_glob && loc_constains) 
    {
        yupc::log_compiler_err("variable \"" + name + ctx->type_annot()->getText() + "\" has already been declared in this scope", ctx->getText());
    }

    this->visit(ctx->type_annot());
    llvm::Type *resolved_type = yupc::comp_units.back()->type_stack.top();

    llvm::Value *val = ctx->var_value() != nullptr 
        ? [&]() 
        {
            this->visit(ctx->var_value()->expr());
            return yupc::comp_units.back()->value_stack.top();
        }()
        : nullptr;

    yupc::var_declare_codegen(name, resolved_type, is_const, is_glob, is_pub, is_ref, val);

    return nullptr;
}

std::any yupc::Visitor::visitAssignment(yupc::YupParser::AssignmentContext *ctx) 
{
    std::string name = ctx->IDENTIFIER()->getText();

    this->visit(ctx->var_value()->expr());
    llvm::Value *val = yupc::comp_units.back()->value_stack.top();

    yupc::assignment_codegen(name, val, ctx->getText());
    
    return nullptr;
}

std::any yupc::Visitor::visitIdentifierExpr(yupc::YupParser::IdentifierExprContext *ctx) 
{
    std::string name = ctx->IDENTIFIER()->getText();

    bool is_glob = yupc::comp_units.back()->global_variables.contains(name);

    bool is_loc = yupc::comp_units.back()->symbol_table.back().contains(name);

    if (!is_glob && !is_loc) 
    {
        yupc::log_compiler_err("symbol \"" + name + "\" is neither a local nor a global variable", ctx->getText());
    }

    yupc::ident_expr_codegen(name, is_glob);
    
    return nullptr;
}
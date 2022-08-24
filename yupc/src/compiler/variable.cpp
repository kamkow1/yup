#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/variable.h>
#include <compiler/compilation_unit.h>

#include <cstddef>
#include <msg/errors.h>
#include <string>

#include <llvm/Support/TypeName.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Alignment.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Metadata.h>

#include <cstdlib>

static std::map<std::string, yupc::Variable> variables;

llvm::AllocaInst *yupc::find_local_variable(std::string name, size_t i,
    std::vector<std::map<std::string, llvm::AllocaInst*>> &symbol_table, std::string text)
{
    if (symbol_table[i].contains(name))
    {
        return symbol_table[i][name];
    }
    else if (i > 0)
    {
        return yupc::find_local_variable(name, --i, symbol_table, text);
    }
    else
    {
        yupc::log_compiler_err("unknown variable " + name, text);
        exit(1);
    }
}

void yupc::ident_expr_codegen(std::string id, bool is_glob) 
{
    if (is_glob)
    {
        llvm::GlobalVariable *global_var = yupc::comp_units.back()->global_variables[id];
        llvm::Type *type = global_var->getValueType();
        llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(type, global_var);
        yupc::comp_units.back()->value_stack.push(load);
    }
    else
    {
        llvm::AllocaInst *variable = yupc::find_local_variable(id, yupc::comp_units.back()->symbol_table.size() - 1,
                                                                yupc::comp_units.back()->symbol_table, "");

        llvm::Type *type = variable->getAllocatedType();
        llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(type, variable);
        yupc::comp_units.back()->value_stack.push(load);
    }
}

void yupc::assignment_codegen(std::string name, llvm::Value *val, std::string text) 
{

    auto var = variables[name];

    if (var.is_const) 
    {
        yupc::log_compiler_err("cannot reassign a constant \"" + name + "\"", text);
        exit(1);
    }

    if (var.is_ref) 
    {
        yupc::log_compiler_err("cannot make a reference point to another variable", text);
        exit(1);
    }

    bool is_local = yupc::comp_units.back()->symbol_table.back().contains(name);
    bool is_global = yupc::comp_units.back()->global_variables.contains(name);

    if (is_local) 
    {
        llvm::AllocaInst *stored = yupc::comp_units.back()->symbol_table.back()[name];
        if (!yupc::check_value_type(val, stored))
        {
            std::string val_type_str = yupc::type_to_string(val->getType());
            std::string stored_type_str = yupc::type_to_string(stored->getType());
            yupc::log_compiler_err("cannot assign type " + val_type_str + " to " + stored_type_str, text);
            exit(1);
        }

        yupc::comp_units.back()->ir_builder->CreateStore(val, stored, false);
        yupc::comp_units.back()->value_stack.pop();
    } 
    else if (is_global) 
    {
        llvm::GlobalVariable *gv = yupc::comp_units.back()->global_variables[name];
        if (!yupc::check_value_type(val, gv))
        {
            std::string gv_type_str = yupc::type_to_string(gv->getValueType());
            std::string val_type_str = yupc::type_to_string(val->getType());
            yupc::log_compiler_err("cannot assign type " + val_type_str + " to " + gv_type_str, text);
            exit(1);
        }

        yupc::comp_units.back()->ir_builder->CreateStore(val, gv, false);
        yupc::comp_units.back()->value_stack.pop();
    }
    else 
    {
        yupc::log_compiler_err("cannot reassign \"" + name + "\" because it doesn't exist", text);
        exit(1);
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
        exit(1);
    }

    bool glob_contains = yupc::comp_units.back()->global_variables.contains(name);

    if (is_glob && glob_contains) 
    {
        yupc::log_compiler_err("global variable \"" + name + ctx->type_annot()->getText() + "\" already exists", ctx->getText());
        exit(1);
    }

    bool loc_constains = false;
    if (yupc::comp_units.back()->symbol_table.size() != 0) 
    {
        yupc::comp_units.back()->symbol_table.back().contains(name);
    }

    if (!is_glob && loc_constains) 
    {
        yupc::log_compiler_err("variable \"" + name + ctx->type_annot()->getText() + "\" has already been declared in this scope", ctx->getText());
        exit(1);
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

    if (val != nullptr)
    {
        if (resolved_type != val->getType())
        {
            std::string rt_str;
            llvm::raw_string_ostream rt_rso(rt_str);
            resolved_type->print(rt_rso);

            std::string vt_str;
            llvm::raw_string_ostream vt_rso(vt_str);
            val->getType()->print(vt_rso);

            yupc::log_compiler_err("mismatch of types " + rt_str + " and " + vt_str, ctx->getText());
            exit(1);
        }
    }

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

    yupc::ident_expr_codegen(name, is_glob);
    
    return nullptr;
}

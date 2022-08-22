#include <compiler/visitor.h>
#include <compiler/ptr_dereference.h>
#include <compiler/compilation_unit.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <msg/errors.h>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

#include <any>

void yupc::ptr_deref_codegen(llvm::Value *value, std::string text) 
{

    if (!value->getType()->isPointerTy()) 
    {
        yupc::log_compiler_err("cannot dereference a non pointer type expression", text);
    }

    llvm::Type *type = value->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::comp_units.back()->ir_builder->CreateLoad(type, value);
    yupc::comp_units.back()->value_stack.push(load);
}

std::any yupc::Visitor::visitPtr_dereference(yupc::YupParser::Ptr_dereferenceContext *ctx) 
{

    this->visit(ctx->expr());
    llvm::Value *value = yupc::comp_units.back()->value_stack.top();

    yupc::ptr_deref_codegen(value, ctx->getText());

    return nullptr;
}

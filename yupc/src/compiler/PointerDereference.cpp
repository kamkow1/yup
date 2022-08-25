#include "compiler/PointerDereference.h"
#include "compiler/Visitor.h"
#include "compiler/CompilationUnit.h"
#include "msg/errors.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

void yupc::PointerDereferenceCodegen(llvm::Value *value, std::string text) 
{

    if (!value->getType()->isPointerTy()) 
    {
        yupc::log_compiler_err("cannot dereference a non pointer type expression", text);
        exit(1);
    }

    llvm::Type *type = value->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(type, value);
    yupc::CompilationUnits.back()->ValueStack.push(load);
}

std::any yupc::Visitor::visitPointerDereference(yupc::YupParser::PointerDereferenceContext *ctx) 
{

    this->visit(ctx->expression());
    llvm::Value *value = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::PointerDereferenceCodegen(value, ctx->getText());
    yupc::CompilationUnits.back()->ValueStack.pop();

    return nullptr;
}

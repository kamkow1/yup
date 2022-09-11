#include "Compiler/PointerDereference.h"
#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Logger.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include <cstddef>
#include <iostream>

llvm::LoadInst *yupc::PointerDereferenceCodegen(llvm::Value *value, size_t line, size_t pos, std::string text) 
{

    if (!value->getType()->isPointerTy()) 
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "cannot dereference a non pointer type expression", 
                                            text, yupc::CompilationUnits.back()->SourceFile);

        exit(1);
    }

    llvm::Type *type = value->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(type, value);
    return load; //yupc::CompilationUnits.back()->ValueStack.push(load);
}

std::any yupc::Visitor::visitPointerDereference(yupc::YupParser::PointerDereferenceContext *ctx) 
{

    this->visit(ctx->expression());
    llvm::Value *value = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::CompilationUnits.back()->ValueStack.pop();
    llvm::LoadInst *load = yupc::PointerDereferenceCodegen(value, ctx->start->getLine(), 
                                    ctx->start->getCharPositionInLine(), ctx->getText());

    yupc::CompilationUnits.back()->ValueStack.push(load);
    return nullptr;
}

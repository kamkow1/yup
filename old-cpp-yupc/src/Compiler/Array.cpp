#include "Compiler/CompilationUnit.h"
#include "Compiler/Constant.h"
#include "Compiler/PointerDereference.h"
#include "Compiler/Visitor.h"
#include "Compiler/Array.h"
#include "Compiler/Type.h"
#include "utils.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/GlobalVariable.h"

#include <cstddef>
#include <string>

llvm::Value *yupc::ArrayPointerIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal,
                                                    size_t line, size_t pos, std::string text)
{
    llvm::Type *gepType = llvm::cast<llvm::AllocaInst>(array)->getAllocatedType();
    llvm::Value *idxGep = yupc::CompilationUnits.back()->IRBuilder->CreateGEP(gepType, array, idxVal);

    llvm::Type *loadType = idxGep->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(loadType, idxGep);
    llvm::LoadInst *deref = yupc::PointerDereferenceCodegen(load, line, pos, text);
    return deref;
}

llvm::Value *yupc::ConstArrayIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal) 
{
    llvm::AllocaInst *arrayAlloca = llvm::cast<llvm::AllocaInst>(array);
    llvm::Type * gepType = arrayAlloca->getAllocatedType();

    std::vector<llvm::Value*> x;
    x.push_back(yupc::IntegerCodegen(0));
    x.push_back(idxVal);
    llvm::Value *idxGep = yupc::CompilationUnits.back()->IRBuilder->CreateGEP(gepType, arrayAlloca, x);

    llvm::Type *loadType = idxGep->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(loadType, idxGep);
    return load;
}

void yupc::ArrayElementAssignmentCodegen(std::string arrayName, size_t idxNestingLvl, std::vector<llvm::Value*> idxVals) 
{
    llvm::AllocaInst *stored = llvm::cast<llvm::AllocaInst>(yupc::CompilationUnits.back()->SymbolTable.back()[arrayName]->ValuePtr);

    llvm::Value *ptr;
    for (size_t i = 0; i < idxNestingLvl; i++) 
    { 
        llvm::Type *gepType = stored->getAllocatedType();
        std::vector<llvm::Value*> x;
        x.push_back(yupc::IntegerCodegen(0));
        x.push_back(idxVals[i]);
        llvm::Value *gep = yupc::CompilationUnits.back()->IRBuilder->CreateGEP(gepType, stored, x);
        
        ptr = gep;
    }

    llvm::Value *val = yupc::CompilationUnits.back()->ValueStack.top();
    yupc::CompilationUnits.back()->IRBuilder->CreateStore(val, ptr);
}

llvm::Constant *yupc::ArrayCodegen(std::vector<llvm::Constant*> elems) 
{
    llvm::Type *elemType = elems[0]->getType();
    llvm::ArrayType *arrayType = llvm::ArrayType::get(elemType, elems.size());
    llvm::Constant *constArray = llvm::ConstantArray::get(arrayType, elems);

    return constArray;
}

std::any yupc::Visitor::visitIndexedAccessExpression(yupc::YupParser::IndexedAccessExpressionContext *ctx) 
{
    std::string name = ctx->expression(0)->getText();
    llvm::Value *array = yupc::CompilationUnits.back()->SymbolTable.back()[name]->ValuePtr;

    if (llvm::cast<llvm::AllocaInst>(array)->getAllocatedType()->isArrayTy())
    {
        this->visit(ctx->expression(1));
        llvm::Value *idxVal = yupc::CompilationUnits.back()->ValueStack.top();
        llvm::Value *val = yupc::ConstArrayIndexedAccessCodegen(array, idxVal);

        yupc::CompilationUnits.back()->ValueStack.push(val);
    }
    else
    {
        this->visit(ctx->expression(1));
        llvm::Value *idxVal = yupc::CompilationUnits.back()->ValueStack.top();
        llvm::Value *val = yupc::ArrayPointerIndexedAccessCodegen(array, idxVal, ctx->start->getLine(), 
                                                   ctx->start->getCharPositionInLine(), ctx->getText());

        yupc::CompilationUnits.back()->ValueStack.push(val);
    }

    return nullptr;
}

std::any yupc::Visitor::visitArrayElementAssignment(yupc::YupParser::ArrayElementAssignmentContext *ctx) 
{
    std::string name = ctx->Identifier()->getText();

    size_t idxNestingLvl = ctx->arrayIndex().size();
    std::vector<llvm::Value*> idxVals;
    for (size_t i = 0; i < idxNestingLvl; i++) 
    {
        this->visit(ctx->arrayIndex(i)->expression());
        idxVals.push_back(yupc::CompilationUnits.back()->ValueStack.top());
    }

    yupc::ArrayElementAssignmentCodegen(name, idxNestingLvl, idxVals);
    return nullptr;
}

std::any yupc::Visitor::visitArray(yupc::YupParser::ArrayContext *ctx) 
{
    std::vector<llvm::Constant*> elems;
    size_t elemCount = ctx->expression().size();
    for (size_t i = 0; i < elemCount; i++) 
    {
        this->visit(ctx->expression(i));
        llvm::Constant *elem = llvm::cast<llvm::Constant>(yupc::CompilationUnits.back()->ValueStack.top());
        elems.push_back(elem);
        yupc::CompilationUnits.back()->ValueStack.pop();
    }

    llvm::Constant *constArray = yupc::ArrayCodegen(elems);
    yupc::CompilationUnits.back()->ValueStack.push(constArray);
    return nullptr;
}

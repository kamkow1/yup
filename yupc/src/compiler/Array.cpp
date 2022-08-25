#include "compiler/CompilationUnit.h"
#include "compiler/Visitor.h"
#include "compiler/Array.h"
#include "compiler/Type.h"
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

void yupc::ConstArrayIndexedAccessCodegen(llvm::Value *array, llvm::Value *idxVal) 
{
    llvm::Type * gepType = array->getType()->getNonOpaquePointerElementType();
    llvm::Value *idxGep = yupc::CompilationUnits.back()->IRBuilder->CreateInBoundsGEP(gepType, array, idxVal);

    yupc::dump_type_to_stdout(idxGep->getType());

    llvm::Type *load_type = idxGep->getType()->getNonOpaquePointerElementType();
    llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(load_type, idxGep);

    yupc::CompilationUnits.back()->ValueStack.push(load);
}

void yupc::ArrayElementAssignmentCodegen(std::string arrayName, size_t idxNestingLvl, std::vector<llvm::Value*> idxVals) 
{
    llvm::AllocaInst *stored = yupc::CompilationUnits.back()->SymbolTable.back()[arrayName];

    llvm::Value *ptr;
    for (size_t i = 0; i < idxNestingLvl; i++) 
    { 
        llvm::Type *gepType = stored->getAllocatedType();
        llvm::Value *gep = yupc::CompilationUnits.back()->IRBuilder->CreateInBoundsGEP(gepType, stored, idxVals[i]);
        ptr = gep;
    }

    llvm::Value *val = yupc::CompilationUnits.back()->ValueStack.top();
    yupc::CompilationUnits.back()->IRBuilder->CreateStore(val, ptr);
}

void yupc::ArrayCodegen(std::vector<llvm::Constant*> elems) 
{
    llvm::Type *elemType = elems[0]->getType();
    llvm::ArrayType *arrayType = llvm::ArrayType::get(elemType, elems.size());
    llvm::Constant *constArray = llvm::ConstantArray::get(arrayType, elems);

    yupc::CompilationUnits.back()->ValueStack.push(constArray);
}

std::any yupc::Visitor::visitIndexedAccessExpression(yupc::YupParser::IndexedAccessExpressionContext *ctx) 
{
    std::string name = ctx->expression(0)->getText();
    llvm::Value *array = yupc::CompilationUnits.back()->SymbolTable.back()[name];

    for (size_t i = 1; i < ctx->expression().size(); i++)
    {
        this->visit(ctx->expression(i));
        llvm::Value *idxVal = yupc::CompilationUnits.back()->ValueStack.top();
        yupc::ConstArrayIndexedAccessCodegen(array, idxVal);
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

    yupc::ArrayCodegen(elems);
    return nullptr;
}

#include "visitor.h"
#include "compiler/type.h"
#include "messaging/errors.h"

using namespace llvm;

std::any Visitor::visitIndexedAccessExpr(YupParser::IndexedAccessExprContext *ctx)
{
    this->visit(ctx->expr(0));
    Value *array = valueStack.top();

    this->visit(ctx->expr(1));
    Value *idxVal = valueStack.top();

    Value *idxGep = irBuilder.CreateGEP(
        array->getType()->getPointerElementType(), array, idxVal);

    valueStack.push(idxGep);

    return nullptr;
}

std::any Visitor::visitArr_elem_assignment(YupParser::Arr_elem_assignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    AllocaInst *stored = symbolTable.top()[name];
    LoadInst *array = irBuilder.CreateLoad(
        stored->getAllocatedType(), stored);

    Value *valToOverride;

    size_t idxNestingLvl = ctx->arr_index().size();
    for (size_t i = 0; i < idxNestingLvl; i++)
    {
        YupParser::Arr_indexContext *idx 
            = ctx->arr_index(i);

        this->visit(idx->expr());
        Value *idxVal = valueStack.top();
        
        Value *idxGep = irBuilder.CreateGEP(
            array->getType()->getPointerElementType(), 
            array, idxVal);

        valToOverride = idxGep;
    }

    this->visit(ctx->var_value()->expr());
    Value *val = valueStack.top();

    irBuilder.CreateStore(val, valToOverride, false);
    valueStack.pop();

    return nullptr;
}

std::any Visitor::visitArray(YupParser::ArrayContext *ctx)
{
    std::vector<Value*> elems;
    size_t elemCount = ctx->expr().size();
    for (size_t i = 0; i < elemCount; i++)
    {
        this->visit(ctx->expr(i));
        Value *elem = valueStack.top();
        elems.push_back(elem);
    }

    Type *elemType = elems[0]->getType();

    Type *i32 = Type::getInt32Ty(context);

    DataLayout dl = module->getDataLayout();

    Constant *sizeOfElement = ConstantInt::get(
        i32, dl.getTypeAllocSize(elemType));
    Constant *length = ConstantInt::get(i32, elemCount);
    Constant *arraySize = ConstantExpr::getMul(sizeOfElement, length);    

    Instruction *arrayMalloc = CallInst::CreateMalloc(
        irBuilder.GetInsertBlock(), 
        elemType, 
        elemType, arraySize,
        nullptr, nullptr, ""); 

    irBuilder.Insert(arrayMalloc);

    for (size_t i = 0; i < elemCount; i++)
    {
        Constant *idx = ConstantInt::get(
            Type::getInt32Ty(context), i);

        Value *idxGep = irBuilder.CreateGEP(
            arrayMalloc->getType()->getPointerElementType(), 
            arrayMalloc, idx);

        irBuilder.CreateStore(elems[i], idxGep);
    }

    valueStack.push(arrayMalloc);

    return nullptr;
}
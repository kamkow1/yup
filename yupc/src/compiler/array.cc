#include "visitor.h"
#include "compiler/array.h"
#include "compiler/type.h"
#include "messaging/errors.h"

using namespace llvm;

void indexedAccessExpr_codegen(Value *array, Value *idxVal)
{
    Value *idxGep = irBuilder.CreateGEP(
        array->getType()->getPointerElementType(), array, idxVal);

    valueStack.push(idxGep);
}

void arrElemAssignment_codegen(std::string arrName, size_t idxNestingLvl, 
                                std::vector<Value*> idxVals)
{
    AllocaInst *stored = symbolTable.top()[arrName];
    LoadInst *array = irBuilder.CreateLoad(
        stored->getAllocatedType(), stored);

    Value *valToOverride;
    for (size_t i = 0; i < idxNestingLvl; i++)
    { 
        Value *idxGep = irBuilder.CreateGEP(
            array->getType()->getPointerElementType(), 
            array, idxVals[i]);

        valToOverride = idxGep;
    }

    //this->visit(ctx->var_value()->expr());
    Value *val = valueStack.top();

    irBuilder.CreateStore(val, valToOverride, false);
    valueStack.pop();
}

void array_codegen(std::vector<Value*> elems, size_t elemCount)
{
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
}

std::any Visitor::visitIndexedAccessExpr(YupParser::IndexedAccessExprContext *ctx)
{
    this->visit(ctx->expr(0));
    Value *array = valueStack.top();

    this->visit(ctx->expr(1));
    Value *idxVal = valueStack.top();

    indexedAccessExpr_codegen(array, idxVal);

    return nullptr;
}

std::any Visitor::visitArr_elem_assignment(YupParser::Arr_elem_assignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    size_t idxNestingLvl = ctx->arr_index().size();
    std::vector<Value*> idxVals;

    for (size_t i = 0; i < idxNestingLvl; i++)
    {
        this->visit(ctx->arr_index(i)->expr());
        idxVals.push_back(valueStack.top());
    }

    arrElemAssignment_codegen(name, idxNestingLvl, idxVals);

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

    array_codegen(elems, elemCount);

    return nullptr;
}
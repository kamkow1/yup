#include "visitor.h"
#include "compiler/type.h"
#include "messaging/errors.h"

using namespace llvm;

std::any Visitor::visitIndexedAccessExpr(YupParser::IndexedAccessExprContext *ctx)
{
    this->visit(ctx->expr(0)); // expr that holds the array
    Value *array = valueStack.top();
    //unsigned index = ctx->expr(0)-

    this->visit(ctx->expr(1));
    Value *index = valueStack.top();

    Value *idxList[2] = 
    {
        ConstantInt::get(index->getType(), 0),
        index
    };

    std::string lbl = "idx_of_" + array->getName().str();
    Value *elem = irBuilder.CreateGEP(array->getType(), array, idxList, lbl);

    valueStack.push(elem);

    return nullptr;
}

std::any Visitor::visitArray(YupParser::ArrayContext *ctx)
{
    std::vector<Constant*> elems;
    size_t elemCount = ctx->expr().size();
    for (int i = 0; i < elemCount; i++)
    {
        this->visit(ctx->expr(i));
        Constant *elem = (Constant*) valueStack.top();
        elems.push_back(elem);
    }

    Type *type = elems[0]->getType();
    Type *arrayType = PointerType::getUnqual(ArrayType::get(type, elemCount));
    Constant* array = ConstantArray::get(
        ArrayType::get(arrayType, elemCount), 
        makeArrayRef(elems));

    valueStack.push(array);

    return nullptr;
}
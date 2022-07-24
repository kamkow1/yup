#include "visitor.h"

using namespace llvm;

std::any Visitor::visitAddr_of(YupParser::Addr_ofContext *ctx)
{
    this->visit(ctx->expr());
    Value *val = valueStack.top();

    AllocaInst *alloca = irBuilder.CreateAlloca(val->getType(), val);
    valueStack.push(alloca);
    return nullptr;
}
#include "visitor.h"
#include "compiler/type.h"
#include "messaging/errors.h"

using namespace llvm;

std::any Visitor::visitArray(YupParser::ArrayContext *ctx)
{
    TypeAnnotation *typeAnnot = 
        std::any_cast<TypeAnnotation*>(this->visit(ctx->type_annot()));
    std::string typeName = typeAnnot->typeName;
    size_t len = ctx->expr().size();
    ArrayType *type = (ArrayType*) resolveType(typeName);

    std::vector<Constant*> elems;
    for (size_t i = 0; i < len; i++)
    {
        YupParser::ExprContext *expr = ctx->expr(i);
        this->visit(expr);
        Constant *value = (Constant*) valueStack.top();

        // type check
        std::string valType;
        raw_string_ostream valRSO(valType);
        value->getType()->print(valRSO);
        if (valRSO.str() != typeName)
        {
            logCompilerError(
                    "type mismatch. array was declared as \"" 
                    + typeName + "[]\" but contains an element of type " 
                    + valType);
        }

        elems.push_back(value);
        valueStack.pop();
    }

    Constant *array = ConstantArray::get(type, makeArrayRef(elems));

    valueStack.push(array);
    return nullptr;
}
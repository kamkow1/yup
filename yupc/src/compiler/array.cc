#include <visitor.h>
#include <compiler/type.h>
#include <messaging/errors.h>

std::any Visitor::visitArray(YupParser::ArrayContext *ctx)
{
    TypeAnnotation* typeAnnot = 
        std::any_cast<TypeAnnotation*>(this->visit(ctx->type_annot()));
    std::string typeName = typeAnnot->typeName;
    size_t len = ctx->expr().size();
    llvm::ArrayType* type = (llvm::ArrayType*) resolveType(typeName, len);

    std::vector<llvm::Constant*> elems;
    for (size_t i = 0; i < len; i++)
    {
        YupParser::ExprContext* expr = ctx->expr(i);
        this->visit(expr);
        llvm::Constant* value = (llvm::Constant*) valueStack.top();

        // type check
        std::string valType;
        llvm::raw_string_ostream valRSO(valType);
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

    llvm::Constant* array = llvm::ConstantArray::get(type, llvm::makeArrayRef(elems));

    valueStack.push(array);
    return nullptr;
}
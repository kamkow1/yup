#include <visitor.h>
#include <compiler/type.h>
#include <messaging/errors.h>
#include <util.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

TypeAnnotation::TypeAnnotation(std::string tn, uint64_t al)
{
    this->typeName = tn;
    this->arrayLen = al;
}

enum BasicType : size_t {
    I32,
    I64,
    FLOAT,
    BOOL,
    VOID,
    CHAR,
    INVALID
};

BasicType resolveBasicType(std::string match)
{
    static const std::map<std::string, BasicType> types
    {
        {"i32", I32},
        {"i64", I64},
        {"float", FLOAT},
        {"void", VOID},
        {"char", CHAR}
    };

    auto itr = types.find(match);
    if (itr != types.end())
    {
        return itr->second;
    }
    else
    {
        return INVALID;
    }
}

llvm::Type* resolveType(std::string typeName, size_t arraySize)
{
    switch (resolveBasicType(typeName))
    {
    case I32:
        return llvm::Type::getInt32Ty(codegenCtx);
    case I64:
        return llvm::Type::getInt64Ty(codegenCtx);
    case FLOAT:
        return llvm::Type::getFloatTy(codegenCtx);
    case BOOL:
        return llvm::Type::getInt8Ty(codegenCtx);
    case VOID:
        return llvm::Type::getVoidTy(codegenCtx);
    case CHAR:
        return llvm::Type::getInt8Ty(codegenCtx);
    default: {
        if (boost::algorithm::contains(typeName, "[]")) // is array
        {
            boost::erase_all(typeName, "[]");
            llvm::Type* primitive = resolveType(typeName);
            BasicType basicType = resolveBasicType(typeName);
            if (basicType != INVALID) // is array of primitives
            {
                return llvm::ArrayType::get(primitive, arraySize);
            }
            else // is array of structs
            {
                logCompilerError("struct arrays are not implemented yet!");
                exit(1);
                /* boost::erase_all(typeName, "[]");
                llvm::StructType* str = (llvm::StructType*) resolveType(typeName);
                return llvm::ArrayType::get(str, arraySize); */
            }
        }
        else // is struct
        {
            logCompilerError("structs are not implemented yet!");
            exit(1);
            return nullptr;
        }
    }
    }

    std::string errorMessage = "couldn't match type \"" + typeName + "\"";
    logCompilerError(errorMessage);
    exit(1);
    return nullptr;
}

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    size_t arrSize = ctx->array_type() != nullptr 
        ? boost::lexical_cast<size_t>(ctx->array_type()->V_INT()->getText())
        : 0;

    TypeAnnotation* p_ta = new TypeAnnotation(name, arrSize);
    return p_ta;
}
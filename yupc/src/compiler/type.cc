#include <visitor.h>
#include <compiler/type.h>
#include <messaging/errors.h>
#include <util.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace llvm;

enum BasicType : size_t {
    I32,
    I64,
    FLOAT,
    BOOL,
    VOID,
    CHAR,

    // pointer types
    P_I32,
    P_I64,
    P_FLOAT,
    P_BOOL,
    P_VOID,
    P_CHAR,

    INVALID
};

BasicType resolveBasicType(std::string match)
{
    static const std::map<std::string, BasicType> types
    {
        { "i32",        I32 },
        { "i64",        I64 },
        { "float",      FLOAT },
        { "void",       VOID },
        { "char",       CHAR },

        {"i32*",        P_I32 },
        {"i64*",        P_I64 },
        {"float*",      P_FLOAT },
        {"void*",       P_VOID },
        {"char*",       P_CHAR }
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

Type* resolveType(std::string typeName)
{
    switch (resolveBasicType(typeName))
    {
    case I32:
        return Type::getInt32Ty(context);
    case I64:
        return Type::getInt64Ty(context);
    case FLOAT:
        return Type::getFloatTy(context);
    case BOOL:
        return Type::getInt8Ty(context);
    case VOID:
        return Type::getVoidTy(context);
    case CHAR:
        return Type::getInt8Ty(context);
        
    case P_I32:
        return Type::getInt32PtrTy(context);
    case P_I64:
        return Type::getInt64PtrTy(context);
    case P_FLOAT:
        return Type::getFloatPtrTy(context);
    case P_BOOL:
        return Type::getInt8PtrTy(context);
    case P_VOID:
        return Type::getInt8PtrTy(context);
    case P_CHAR:
        return Type::getInt8PtrTy(context);

    default: {
        std::string errorMessage = "couldn't match type \"" + typeName + "\"";
        logCompilerError(errorMessage);
        exit(1);
        return nullptr;
    }
    }
}

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    if (ctx->ASTERISK() != nullptr)
    {
        name += "*";
    }

    if (ctx->AMPERSAND() != nullptr)
    {
        name += "&";
    }

    TypeAnnotation ta = TypeAnnotation{name};
    return ta;
}
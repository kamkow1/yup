#include <visitor.h>
#include <compiler/type.h>
#include <messaging/errors.h>
#include <util.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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

llvm::Type* resolveType(std::string typeName)
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

    case P_I32:
        return llvm::Type::getInt32PtrTy(codegenCtx);
    case P_I64:
        return llvm::Type::getInt64PtrTy(codegenCtx);
    case P_FLOAT:
        return llvm::Type::getFloatPtrTy(codegenCtx);
    case P_BOOL:
        return llvm::Type::getInt8PtrTy(codegenCtx);
    case P_VOID:
        return llvm::Type::getInt8PtrTy(codegenCtx);
    case P_CHAR:
        return llvm::Type::getInt8PtrTy(codegenCtx);

    default: {
        std::cout << "def\n";

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
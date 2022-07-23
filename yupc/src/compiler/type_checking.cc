#include "compiler/type.h"
#include "messaging/errors.h"
#include "boost/algorithm/string.hpp"
#include "visitor.h"
#include "string.h"

using namespace llvm;
using namespace boost;

enum BasicType : size_t {
    I32,
    I64,
    FLOAT,
    BOOL,
    VOID,
    CHAR,
    PTR,
    INVALID
};

Type *resolvePointerType(Type *base)
{
    return PointerType::get(base, 0);
}

BasicType resolveBasicType(std::string match)
{
    static const std::map<std::string, BasicType> types
    {
        { "i32",        I32 },
        { "i64",        I64 },
        { "float",      FLOAT },
        { "void",       VOID },
        { "char",       CHAR },
        { "bool",       BOOL}
    };

    auto itr = types.find(match);
    if (itr != types.end())
    {
        return itr->second;
    }
    else if (algorithm::contains(match, "*")) // pointer type
    {
        return PTR;
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
    case PTR: {
        std::string originalType = typeName;
        erase_all(originalType, "*");
        Type *baseType = resolveType(originalType);

        int asterCount =  std::count_if(
            typeName.begin(), 
            typeName.end(), 
            []( char c )
            {
                return c == '*';
            }
        );

        Type *type;
        for (int i = 0; i < asterCount; i++)
        {
            type = resolvePointerType(baseType);
        }

        return type;
    }

    case INVALID: {
        std::string errorMessage = "couldn't match type \"" + typeName + "\"";
        logCompilerError(errorMessage);
        exit(1);
        return nullptr;
    }
    }

    return nullptr;
}

std::string getReadableTypeName(std::string typeName)
{    
    return typeName;
}

void checkValueType(Value *val, std::string name)
{
    std::string exprType;
    raw_string_ostream rso(exprType);
    val->getType()->print(rso);
    exprType = getReadableTypeName(rso.str());

    Value *ogVal = symbolTable.top()[name];
    std::string ogType;
    raw_string_ostream ogRso(ogType);
    ogVal->getType()->print(ogRso);
    ogType = getReadableTypeName(ogRso.str());

    if ((ogType == "bool" || ogType == "char") || exprType == "i8")
    {
        return;
    }

    if (exprType != ogType)
    {
        logCompilerError("mismatch of types \"" + ogType 
            + "\" and \"" + exprType + "\"");
        exit(1);
    }
}
#include "visitor.h"
#include "util.h"
#include "compiler/type.h"
#include "messaging/errors.h"
#include "boost/algorithm/string.hpp"
#include "string.h"

using namespace llvm;
using namespace boost;

static Type *resolveArrayType(Type *base)
{
    return ArrayType::get(base, 3);
}

static Type *resolvePointerType(Type *base)
{
    return PointerType::get(base, 0);
}

static std::map<std::string, size_t> types
{
    { "i32",    1 },
    { "i64",    2 },
    { "float",  3 },
    { "bool",   4 },
    { "void",   5 },
    { "char",   6 },
};

static size_t resolveBasicType(std::string match)
{
    auto itr = types.find(match);
    if (itr != types.end())
    {
        return itr->second;
    }
    else
    {
        return SIZE_MAX;
    }
}

void appendTypeID(size_t n, std::string idStr)
{
    types[idStr] = n;
}

Type* resolveType(std::string typeName) 
{
    switch (resolveBasicType(typeName))
    {
        case 1: // i32
            return Type::getInt32Ty(context);
        case 2: // i64
            return Type::getInt64Ty(context);
        case 3: // float
            return Type::getFloatTy(context);
        case 4: // bool
            return Type::getInt8Ty(context);
        case 5: // void
            return Type::getVoidTy(context);
        case 6: // char
            return Type::getInt8Ty(context);

        case SIZE_MAX:
        {
            std::string baseStr = typeName;
            algorithm::erase_all(baseStr, "[");
            algorithm::erase_all(baseStr, "]");
            algorithm::erase_all(baseStr, "*");

            Type *base = resolveType(baseStr);

            std::string suffixes = typeName;
            algorithm::erase_all(suffixes, baseStr);
            algorithm::erase_all(suffixes, "]"); // * - pointer type, [ - array type

            for (size_t i = 0; i < suffixes.size(); i++)
            {
                char c = suffixes[i];
                switch (c)
                {
                    case '*':
                        base = resolvePointerType(base);
                        break;
                    case '[':
                        base = resolveArrayType(base);
                        break;
                }
            }

            return base;
        }
    }

    std::string errorMessage = "couldn't match type \"" + typeName + "\"";
    logCompilerError(errorMessage);
    exit(1);

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

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->type_name()->getText();

    TypeAnnotation ta = TypeAnnotation{name};
    return ta;
}
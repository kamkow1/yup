#include "compiler/visitor.h"
#include "compiler/type.h"
#include "messaging/errors.h"
#include "util.h"

#include "boost/algorithm/string.hpp"

#include "string.h"

using namespace llvm;
using namespace boost;
using namespace YupCompiler;

static Type *resolve_ptr_type(Type *base)
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

static size_t resolve_basic_type(std::string match)
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

void appendTypeID(size_t n, std::string id_str)
{
    types[id_str] = n;
}

Type* resolve_type(std::string type_name) 
{
    switch (resolve_basic_type(type_name))
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
            std::string base_str = type_name;
            algorithm::erase_all(base_str, "*");

            Type *base = resolve_type(base_str);

            std::string suffixes = type_name;
            algorithm::erase_all(suffixes, base_str);
            
            for (size_t i = 0; i < suffixes.size(); i++)
            {
                char c = suffixes[i];

                switch (c)
                {
                    case '*':
                        base = resolve_ptr_type(base);
                        break;                  
                }
            }

            return base;
        }
    }

    log_compiler_err("couldn't match type \"" + type_name + "\"");
    exit(1);

    return nullptr;
}

std::string get_readable_type_name(std::string type_name)
{    
    return type_name;
}

void check_value_type(Value *val, std::string name)
{
    std::string expr_type;
    raw_string_ostream rso(expr_type);
    val->getType()->print(rso);
    expr_type = get_readable_type_name(rso.str());

    Value *og_val = symbol_table.top()[name];
    std::string og_type;
    raw_string_ostream og_rso(og_type);
    og_val->getType()->print(og_rso);
    og_type = get_readable_type_name(og_rso.str());

    if ((og_type == "bool" || og_type == "char") || expr_type == "i8")
    {
        return;
    }

    if (expr_type != og_type)
    {
        log_compiler_err("mismatch of types \"" + og_type 
            + "\" and \"" + expr_type + "\"");
        exit(1);
    }
}

std::any Visitor::visitType_annot(Parser::YupParser::Type_annotContext *ctx)
{
    std::string base = ctx->type_name()->IDENTIFIER()->getText();
    Type *type_base = resolve_type(base);

    size_t ext_len = ctx->type_name()->type_ext().size();
    for (size_t i = 0; i < ext_len; i++)
    {
        Parser::YupParser::Type_extContext *ext 
            = ctx->type_name()->type_ext(i);

        if (ext->ASTERISK() != nullptr)
        {
            base += "*";
            type_base = resolve_type(base);
        } 
    }

    return type_base;
}

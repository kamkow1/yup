#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>
#include <msg/errors.h>
#include <util.h>

#include <boost/algorithm/string.hpp>

#include <string>

using namespace llvm;
using namespace boost;
using namespace yupc;
using namespace yupc::msg::errors;

namespace cv = compiler::visitor;
namespace ct = compiler::type;
namespace com_un = compiler::compilation_unit;

static Type *ct::resolve_ptr_type(Type *base) {
    return PointerType::get(base, 0);
}

static std::map<std::string, size_t> types {
    { "i32",    1 },
    { "i64",    2 },
    { "float",  3 },
    { "bool",   4 },
    { "void",   5 },
    { "char",   6 },
};

static size_t ct::resolve_basic_type(std::string match) {
    auto itr = types.find(match);
    if (itr != types.end()) {
        return itr->second;
    } else {
        return SIZE_MAX;
    }
}

void ct::appendTypeID(size_t n, std::string id_str) {
    types[id_str] = n;
}

Type* ct::resolve_type(std::string type_name) {
    switch (ct::resolve_basic_type(type_name)) {
        case 1: // i32
            return Type::getInt32Ty(
                com_un::comp_units.top().context);
        case 2: // i64
            return Type::getInt64Ty(
                com_un::comp_units.top().context);
        case 3: // float
            return Type::getFloatTy(
                com_un::comp_units.top().context);
        case 4: // bool
            return Type::getInt8Ty(
                com_un::comp_units.top().context);
        case 5: // void
            return Type::getVoidTy(
                com_un::comp_units.top().context);
        case 6: // char
            return Type::getInt8Ty(
                com_un::comp_units.top().context);

        case SIZE_MAX: {
            std::string base_str = type_name;
            algorithm::erase_all(base_str, "*");

            Type *base = resolve_type(base_str);

            std::string suffixes = type_name;
            algorithm::erase_all(suffixes, base_str);
            
            for (size_t i = 0; i < suffixes.size(); i++) {
                char c = suffixes[i];

                switch (c) {
                    case '*':
                        base = ct::resolve_ptr_type(base);
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

std::string ct::get_readable_type_name(std::string type_name) {    
    return type_name;
}

void ct::check_value_type(Value *val, std::string name) {
    std::string expr_type;
    raw_string_ostream rso(expr_type);

    val->getType()->print(rso);
    expr_type = ct::get_readable_type_name(rso.str());

    Value *og_val;
    bool is_local = com_un::comp_units.top()
        .symbol_table.top().contains(name);

    if (is_local) {
        og_val = com_un::comp_units.top()
            .symbol_table.top()[name];
    } else {
        og_val = com_un::comp_units.top()
            .global_variables[name];
    }

    std::string og_type;
    raw_string_ostream og_rso(og_type);

    og_val->getType()->print(og_rso);
    og_type = ct::get_readable_type_name(og_rso.str());

    if (!is_local) {
        og_type.pop_back();
    }

    if ((og_type == "bool" || og_type == "char") || expr_type == "i8") {
        return;
    }

    if (expr_type != og_type) {
        log_compiler_err("mismatch of types \"" + og_type 
            + "\" and \"" + expr_type + "\"");
        exit(1);
    }
}

std::any cv::Visitor::visitType_annot(parser::YupParser::Type_annotContext *ctx) {
    std::string base = ctx->type_name()->IDENTIFIER()->getText();
    Type *type_base = ct::resolve_type(base);

    size_t ext_len = ctx->type_name()->type_ext().size();
    for (size_t i = 0; i < ext_len; i++) {
        parser::YupParser::Type_extContext *ext 
            = ctx->type_name()->type_ext(i);

        if (ext->ASTERISK() != nullptr) {
            base += "*";
            type_base = ct::resolve_type(base);
        } 
    }

    return type_base;
}

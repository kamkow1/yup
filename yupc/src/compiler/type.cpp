#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>
#include <msg/errors.h>
#include <util.h>

#include <boost/algorithm/string.hpp>

#include <any>
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
    { "i32",    ct::I32_TYPE },
    { "i64",    ct::I64_TYPE },
    { "float",  ct::FLOAT_TYPE },
    { "bool",   ct::BOOL_TYPE },
    { "void",   ct::VOID_TYPE },
    { "char",   ct::CHAR_TYPE },
    { "i8",     ct::I8_TYPE }
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

    for (auto i = 0; i < com_un::comp_units.back()->alias_types.size(); i++) {
        
        auto *alias_type = com_un::comp_units.back()->alias_types[i];

        if (alias_type->is_public) {

            if (alias_type->type_name == type_name) {
                type_name = com_un::comp_units.back()->alias_types[i]->destination;
            }
        }
    }

    switch (ct::resolve_basic_type(type_name)) {
        case ct::I32_TYPE: // i32
            return Type::getInt32Ty(*com_un::comp_units.back()->context);
        case ct::I64_TYPE: // i64
            return Type::getInt64Ty(*com_un::comp_units.back()->context);
        case ct::I8_TYPE:
            return Type::getInt8Ty(*com_un::comp_units.back()->context);
        case ct::FLOAT_TYPE: // float
            return Type::getFloatTy(*com_un::comp_units.back()->context);
        case ct::BOOL_TYPE: // bool
            return Type::getInt8Ty(*com_un::comp_units.back()->context);
        case ct::VOID_TYPE: // void
            return Type::getVoidTy(*com_un::comp_units.back()->context);
        case ct::CHAR_TYPE: // char
            return Type::getInt8Ty(*com_un::comp_units.back()->context);

        case SIZE_MAX: {
            auto base_str = type_name;
            algorithm::erase_all(base_str, "*");

            auto *base = resolve_type(base_str);

            auto suffixes = type_name;
            algorithm::erase_all(suffixes, base_str);
            
            for (auto i = 0; i < suffixes.size(); i++) {
                auto c = suffixes[i];

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
    auto is_local = com_un::comp_units.back()->symbol_table.back().contains(name);

    if (is_local) {
        og_val = com_un::comp_units.back()->symbol_table.back()[name];
    } else {
        og_val = com_un::comp_units.back()->global_variables[name];
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
        log_compiler_err("mismatch of types \"" + og_type + "\" and \"" + expr_type + "\"");
        exit(1);
    }
}

std::any cv::Visitor::visitType_decl(parser::YupParser::Type_declContext *ctx) {
    if (ctx->type_def()->type_alias() != nullptr) {

        auto alias_name = ctx->IDENTIFIER()->getText();
        auto old_name = ctx->type_def()->type_alias()->type_annot()->getText();

        auto *type = new ct::AliasType;
        type->type_name = alias_name;
        type->destination = old_name;
        type->is_public = ctx->PUBSYM() != nullptr;

        com_un::comp_units.back()->alias_types.push_back(type);
    }

    return nullptr;
}

std::any cv::Visitor::visitType_annot(parser::YupParser::Type_annotContext *ctx) {
    
    auto base = ctx->type_name()->IDENTIFIER()->getText();
    auto *type_base = ct::resolve_type(base);

    auto ext_len = ctx->type_name()->type_ext().size();
    for (auto i = 0; i < ext_len; i++) {
        auto *ext = ctx->type_name()->type_ext(i);

        if (ext->ASTERISK() != nullptr) {
            base += "*";
            type_base = ct::resolve_type(base);
        } 
    }

    return type_base;
}

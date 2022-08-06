#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/constant.h>
#include <compiler/type.h>
#include <msg/errors.h>

#include <llvm/IR/Constants.h>

#include <boost/lexical_cast.hpp>

using namespace llvm;
using namespace boost;
using namespace yupc;
using namespace yupc::msg::errors;

namespace cv = compiler::visitor;
namespace cc = compiler::constant;
namespace ct = compiler::type;
namespace com_un = compiler::compilation_unit;

void cc::int_codegen(int64_t value) {
    ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? ConstantInt::get(Type::getInt64Ty(
                *com_un::comp_units[com_un::current_comp_unit_id]->context), value)

            : ConstantInt::get(Type::getInt32Ty(
                *com_un::comp_units[com_un::current_comp_unit_id]->context), value);

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(constant);
}

void cc::float_codegen(float value) {
    Value *constant = ConstantFP::get(
        *com_un::comp_units[com_un::current_comp_unit_id]
            ->context, APFloat(value));

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(constant);
}

void cc::bool_codegen(bool value) {
    Value *constant = ConstantInt::get(Type::getInt8Ty(
        *com_un::comp_units[com_un::current_comp_unit_id]->context), value);

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(constant);
}

void cc::char_codegen(std::string text) {
    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    Value *constant = ConstantInt::get(Type::getInt8Ty(
            *com_un::comp_units[com_un::current_comp_unit_id]->context), *cstr);

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(constant);

    delete []cstr;
}

void cc::string_codegen(std::string text) {
    Constant *gstrptr = com_un::comp_units[com_un::current_comp_unit_id]
        ->ir_builder.CreateGlobalStringPtr(StringRef(text));

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(gstrptr);
}

void cc::null_codegen(std::string type_name) {
    Type *ptype = ct::resolve_type(type_name);

    Constant *nullp = ConstantPointerNull::getNullValue(ptype);

    com_un::comp_units[com_un::current_comp_unit_id]
        ->value_stack.push(nullp);
}

std::any cv::Visitor::visitConstant(parser::YupParser::ConstantContext *ctx) {
    if (ctx->V_INT() != nullptr) {
        std::string text = ctx->V_INT()->getText();
        int64_t value = lexical_cast<int64_t>(text.c_str());
        
        cc::int_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_FLOAT() != nullptr) {
        std::string text = ctx->V_FLOAT()->getText();
        float value = std::atof(text.c_str());
        
        cc::float_codegen(value);

        return nullptr;
    }

    if (ctx->V_BOOL() != nullptr) {
        std::string text = ctx->V_BOOL()->getText();
        bool value = text == "True";

        cc::bool_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_CHAR() != nullptr) {
        std::string text = ctx->V_CHAR()->getText();

        cc::char_codegen(text);
        
        return nullptr;
    }

    if (ctx->V_STRING() != nullptr) {
        std::string text = ctx->V_STRING()->getText();
        text.erase(0, 1);
        text.erase(text.size() - 1);

        cc::string_codegen(text);

        return nullptr;
    }

    if (ctx->null_const() != nullptr) {
        std::string type_name = ctx->null_const()
                                    ->type_name()
                                    ->getText();

        cc::null_codegen(type_name);

        return nullptr;
    }

    log_compiler_err("couldn't match type and create a constant");
    exit(1);
}

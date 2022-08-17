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

    auto *constant = value > INT32_MAX || value < INT32_MIN
            ? ConstantInt::get(Type::getInt64Ty(*com_un::comp_units.back()->context), value)
            : ConstantInt::get(Type::getInt32Ty(*com_un::comp_units.back()->context), value);

    com_un::comp_units.back()->value_stack.push(constant);
}

void cc::float_codegen(float value) {

    auto *constant = ConstantFP::get(*com_un::comp_units.back()->context, APFloat(value));

    com_un::comp_units.back()->value_stack.push(constant);
}

void cc::bool_codegen(bool value) {

    auto *constant = ConstantInt::get(Type::getInt8Ty(*com_un::comp_units.back()->context), value);

    com_un::comp_units.back()->value_stack.push(constant);
}

void cc::char_codegen(std::string text) {

    auto *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    auto *constant = ConstantInt::get(Type::getInt8Ty(*com_un::comp_units.back()->context), *cstr);

    com_un::comp_units.back()->value_stack.push(constant);

    delete []cstr;
}

void cc::string_codegen(std::string text) {

    auto *gstrptr = com_un::comp_units.back()->ir_builder->CreateGlobalStringPtr(StringRef(text));

    com_un::comp_units.back()->value_stack.push(gstrptr);
}

void cc::null_codegen(std::string type_name) {

    auto *ptype = ct::resolve_type(type_name);

    auto *nullp = ConstantPointerNull::getNullValue(ptype);

    com_un::comp_units.back()->value_stack.push(nullp);
}

std::any cv::Visitor::visitConstant(parser::YupParser::ConstantContext *ctx) {
    
    if (ctx->V_INT() != nullptr) {
        auto text = ctx->V_INT()->getText();
        auto value = lexical_cast<int64_t>(text.c_str());
        
        cc::int_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_FLOAT() != nullptr) {
        auto text = ctx->V_FLOAT()->getText();
        auto value = std::atof(text.c_str());
        
        cc::float_codegen(value);

        return nullptr;
    }

    if (ctx->V_BOOL() != nullptr) {
        auto text = ctx->V_BOOL()->getText();
        auto value = text == "True";

        cc::bool_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_CHAR() != nullptr) {
        auto text = ctx->V_CHAR()->getText();

        cc::char_codegen(text);
        
        return nullptr;
    }

    if (ctx->V_STRING() != nullptr) {
        auto text = ctx->V_STRING()->getText();
        std::cout << text << "\n";
        text.erase(0, 1);
        text.erase(text.size() - 1);

        cc::string_codegen(text);

        return nullptr;
    }

    if (ctx->null_const() != nullptr) {
        auto type_name = ctx->null_const()->type_name()->getText();

        cc::null_codegen(type_name);

        return nullptr;
    }

    log_compiler_err("couldn't match type and create a constant", ctx->getText());
    exit(1);
}

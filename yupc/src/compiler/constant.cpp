#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/constant.h>
#include <compiler/type.h>
#include <llvm/IR/Type.h>
#include <msg/errors.h>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>

#include <boost/lexical_cast.hpp>
#include <string>
#include <cstdint>


void yupc::int_codegen(int64_t value) 
{

    llvm::ConstantInt *constant = value > INT32_MAX || value < INT32_MIN
            ? llvm::ConstantInt::get(llvm::Type::getInt64Ty(*yupc::comp_units.back()->context), value)
            : llvm::ConstantInt::get(llvm::Type::getInt32Ty(*yupc::comp_units.back()->context), value);

    yupc::comp_units.back()->value_stack.push(constant);
}

void yupc::float_codegen(float value) 
{

    llvm::ConstantFP *constant = llvm::ConstantFP::get(*yupc::comp_units.back()->context, llvm::APFloat(value));

    yupc::comp_units.back()->value_stack.push(constant);
}

void yupc::bool_codegen(bool value) 
{

    llvm::ConstantInt *constant = llvm::ConstantInt::get(llvm::Type::getInt8Ty(*yupc::comp_units.back()->context), value);

    yupc::comp_units.back()->value_stack.push(constant);
}

void yupc::char_codegen(std::string text) 
{

    char *cstr = new char[text.length() + 1];
    strcpy(cstr, &text.c_str()[1]);

    llvm::ConstantInt *constant = llvm::ConstantInt::get(llvm::Type::getInt8Ty(*yupc::comp_units.back()->context), *cstr);

    yupc::comp_units.back()->value_stack.push(constant);

    delete []cstr;
}

void yupc::string_codegen(std::string text) 
{

    std::string new_string = text.substr(1, text.size() - 2);
    llvm::Constant *gstrptr = yupc::comp_units.back()->ir_builder->CreateGlobalStringPtr(llvm::StringRef(new_string));

    yupc::comp_units.back()->value_stack.push(gstrptr);
}

void yupc::null_codegen(std::string type_name) 
{

    llvm::Type *ptype = yupc::resolve_type(type_name);

    llvm::Constant *nullp = llvm::ConstantPointerNull::getNullValue(ptype);

    yupc::comp_units.back()->value_stack.push(nullp);
}

std::any yupc::Visitor::visitConstant(yupc::YupParser::ConstantContext *ctx) 
{
    
    if (ctx->V_INT() != nullptr) 
    {
        std::string text = ctx->V_INT()->getText();
        int64_t value = boost::lexical_cast<int64_t>(text.c_str());
        
        yupc::int_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_FLOAT() != nullptr) 
    {
        std::string text = ctx->V_FLOAT()->getText();
        double value = std::atof(text.c_str());
        
        yupc::float_codegen(value);

        return nullptr;
    }

    if (ctx->V_BOOL() != nullptr) 
    {
        std::string text = ctx->V_BOOL()->getText();
        bool value = text == "True";

        yupc::bool_codegen(value);
        
        return nullptr;
    }

    if (ctx->V_CHAR() != nullptr) 
    {
        std::string text = ctx->V_CHAR()->getText();

        yupc::char_codegen(text);
        
        return nullptr;
    }

    if (ctx->V_STRING() != nullptr) 
    {
        std::string text = ctx->V_STRING()->getText();
        text.erase(0, 1);
        text.erase(text.size() - 1);

        yupc::string_codegen(text);

        return nullptr;
    }

    if (ctx->null_const() != nullptr) 
    {
        std::string type_name = ctx->null_const()->type_name()->getText();

        yupc::null_codegen(type_name);

        return nullptr;
    }

    yupc::log_compiler_err("couldn't match type and create a constant", ctx->getText());
    exit(1);
}

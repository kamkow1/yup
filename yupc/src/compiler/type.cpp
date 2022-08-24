#include <compiler/visitor.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>

#include <llvm/Support/raw_ostream.h>
#include <parser/YupParser.h>
#include <lexer/YupLexer.h>

#include <msg/errors.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <any>
#include <cstddef>
#include <cstdint>
#include <string>
#include <map>

llvm::Type *yupc::resolve_ptr_type(llvm::Type *base) 
{
    return llvm::PointerType::get(base, 0);
}

static std::map<std::string, size_t> types 
{
    { "i32",    yupc::LLVM_I32_TYPE },
    { "i64",    yupc::LLVM_I64_TYPE },
    { "float",  yupc::LLVM_FLOAT_TYPE },
    { "void",   yupc::LLVM_VOID_TYPE },
    { "i8",     yupc::LLVM_I8_TYPE } // for LLVM
};

size_t yupc::resolve_basic_type(std::string match) 
{
    auto itr = types.find(match);
    if (itr != types.end()) {
        return itr->second;
    } else {
        return SIZE_MAX;
    }
}

void appendTypeID(size_t n, std::string id_str) 
{
    types[id_str] = n;
}

std::string yupc::type_to_string(llvm::Type *type)
{
    std::string str;
    llvm::raw_string_ostream rso(str);
    type->print(rso);
    return str;
}

llvm::Type* yupc::resolve_type(std::string type_name, llvm::LLVMContext &ctx_ref) 
{
    for (size_t i = 0; i < yupc::comp_units.back()->alias_types.size(); i++) 
    {
        yupc::AliasType *alias_type = yupc::comp_units.back()->alias_types[i];

        if (alias_type->is_public) 
        {

            if (alias_type->type_name == type_name) {
                type_name = yupc::comp_units.back()->alias_types[i]->destination;
            }
        }
    }

    switch (yupc::resolve_basic_type(type_name)) 
    {
        case yupc::LLVM_I32_TYPE: // i32
            return llvm::Type::getInt32Ty(ctx_ref);

        case yupc::LLVM_I64_TYPE: // i64
            return llvm::Type::getInt64Ty(ctx_ref);

        case yupc::LLVM_FLOAT_TYPE: // float
            return llvm::Type::getFloatTy(ctx_ref);

        case yupc::LLVM_VOID_TYPE: // void
            return llvm::Type::getVoidTy(ctx_ref);

        case yupc::LLVM_I8_TYPE:
            return llvm::Type::getInt8Ty(ctx_ref);

        case SIZE_MAX: 
        {
            std::string base_str = type_name;
            boost::algorithm::erase_all(base_str, "*");

            llvm::Type *base = yupc::resolve_type(base_str, ctx_ref);

            std::string suffixes = type_name;
            boost::algorithm::erase_all(suffixes, base_str);
            
            for (size_t i = 0; i < suffixes.size(); i++) 
            {
                auto c = suffixes[i];

                if (c == '*') 
                {
                    base = yupc::resolve_ptr_type(base);
                }
            }

            return base;
        }
    }

    yupc::log_compiler_err("couldn't match type \"" + type_name + "\"", "");
    exit(1);

    return nullptr;
}

llvm::Type *yupc::resolve_fixed_array_type(llvm::Type *base, uint64_t size) 
{
    llvm::ArrayType *array_type = llvm::ArrayType::get(base, size);    
    
    return array_type;
}

std::string yupc::get_readable_type_name(std::string type_name) 
{    
    return type_name;
}

bool yupc::check_value_type(llvm::Value *val1, llvm::Value *val2) 
{
    if (val1->getType()->isPointerTy())
    {
        return val1->getType()->getTypeID() == val2->getType()->getTypeID();
    }
    else
    {
        return val1->getType()->getPointerTo()->getTypeID() == val2->getType()->getTypeID();
    }
    /*std::string expr_type;
    llvm::raw_string_ostream rso(expr_type);

    val->getType()->print(rso);
    expr_type = yupc::get_readable_type_name(rso.str());

    llvm::Value *og_val;
    bool is_local = yupc::comp_units.back()->symbol_table.back().contains(name);

    if (is_local) 
    {
        og_val = yupc::comp_units.back()->symbol_table.back()[name];
    } 
    else 
    {
        og_val = yupc::comp_units.back()->global_variables[name];
    }

    std::string og_type;
    llvm::raw_string_ostream og_rso(og_type);

    og_val->getType()->print(og_rso);
    og_type = yupc::get_readable_type_name(og_rso.str());

    if (!is_local) 
    {
        og_type.pop_back();
    }

    if (expr_type != og_type) 
    {
        yupc::log_compiler_err("mismatch of types \"" + og_type + "\" and \"" + expr_type + "\"", "");
        exit(1);
    }*/
}

std::any yupc::Visitor::visitTypeNameExpr(yupc::YupParser::TypeNameExprContext *ctx)
{
    return this->visit(ctx->type_name_expr());
}

std::any yupc::Visitor::visitType_name_expr(yupc::YupParser::Type_name_exprContext *ctx)
{
    return this->visit(ctx->type_name());
}

std::any yupc::Visitor::visitType_decl(yupc::YupParser::Type_declContext *ctx) 
{
    if (ctx->type_def()->type_alias() != nullptr) 
    {
        std::string alias_name = ctx->IDENTIFIER()->getText();
        std::string old_name = ctx->type_def()->type_alias()->type_name()->getText();

        auto *type = new AliasType;
        type->type_name = alias_name;
        type->destination = old_name;
        type->is_public = ctx->PUBSYM() != nullptr;

        yupc::comp_units.back()->alias_types.push_back(type);
    }

    return nullptr;
}

std::any yupc::Visitor::visitTypeCastExpr(yupc::YupParser::TypeCastExprContext *ctx) 
{
    
    this->visit(ctx->expr(0));
    llvm::Type *dest_type = yupc::comp_units.back()->type_stack.top();

    this->visit(ctx->expr(1));
    llvm::Value *val = yupc::comp_units.back()->value_stack.top();

    llvm::Value *cast;

    if (val->getType()->isIntegerTy() && dest_type->isIntegerTy())
    {
        cast = yupc::comp_units.back()->ir_builder->CreateIntCast(val, dest_type, true);
    }
    else if (val->getType()->isIntegerTy() && dest_type->isPointerTy())
    {
        cast = yupc::comp_units.back()->ir_builder->CreateIntToPtr(val, dest_type);
    }
    else if (val->getType()->isPointerTy() && dest_type->isPointerTy())
    {
        cast = yupc::comp_units.back()->ir_builder->CreatePointerCast(val, dest_type);
    }
    else
    {
        cast = yupc::comp_units.back()->ir_builder->CreateBitCast(val, dest_type);
    }

    comp_units.back()->value_stack.push(cast);

    return nullptr;
}

std::any yupc::Visitor::visitType_name(yupc::YupParser::Type_nameContext *ctx)
{
    std::string base = ctx->IDENTIFIER()->getText();

    for (size_t i = 0; i < yupc::comp_units.back()->alias_types.size(); i++) 
    {
        yupc::AliasType *at = yupc::comp_units.back()->alias_types[i];
        
        if (at->type_name == base)
        {
            base = at->destination;
        }
    }    

    llvm::Type *type_base = yupc::resolve_type(base, yupc::comp_units.back()->module->getContext());

    size_t ext_len = ctx->type_ext().size();
    for (size_t i = 0; i < ext_len; i++) 
    {
        yupc::YupParser::Type_extContext *ext = ctx->type_ext(i);

        if (ext->ASTERISK() != nullptr) 
        {
            base += "*";
            type_base = yupc::resolve_type(base, yupc::comp_units.back()->module->getContext());
        }

        if (ext->array_type_ext() != nullptr) 
        {
            if (dynamic_cast<yupc::YupParser::ConstantExprContext*>(ext->array_type_ext()->expr())) 
            {
                unsigned long elem_count = boost::lexical_cast<uint64_t>(ext->array_type_ext()->expr()->getText());

                type_base = yupc::resolve_fixed_array_type(type_base, elem_count);
            }
        }
    }

    comp_units.back()->type_stack.push(type_base);

    return nullptr;
}

std::any yupc::Visitor::visitType_annot(yupc::YupParser::Type_annotContext *ctx) 
{
    return this->visit(ctx->type_name());
}

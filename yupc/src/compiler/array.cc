#include "compiler/visitor.h"
#include "compiler/array.h"
#include "compiler/type.h"
#include "msg/errors.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "cstdint"

#include "boost/lexical_cast.hpp"

using namespace llvm;
using namespace yupc;
using namespace boost;

namespace cv = compiler::visitor;
namespace ca = compiler::array;
namespace ct = compiler::type;

Instruction *ca::create_array_malloc(Type *elem_type, size_t elem_count, DataLayout dl)
{
    //Type *elem_type = elems[0]->getType();

    Type *i32 = Type::getInt32Ty(cv::context);

    //DataLayout dl = compiler::visitor::module->getDataLayout();

    Constant *size_of_element = ConstantInt::get(
        i32, dl.getTypeAllocSize(elem_type));

    Constant *length = ConstantInt::get(i32, elem_count);
    Constant *array_size = ConstantExpr::getMul(size_of_element, length);    

    Instruction *array_malloc = CallInst::CreateMalloc(
        cv::ir_builder.GetInsertBlock(), 
        elem_type, 
        elem_type, array_size,
        nullptr, nullptr, ""); 

    cv::ir_builder.Insert(array_malloc);

    return array_malloc;
}

void ca::indexed_access_expr_codegen(Value *array, Value *idxVal)
{
    Value *idx_gep = cv::ir_builder.CreateGEP(
        array->getType()->getPointerElementType(), array, idxVal);

    cv::value_stack.push(idx_gep);
}

void ca::arr_elem_assignment_codegen(std::string arr_name, 
                                size_t idx_nesting_lvl, 
                                std::vector<Value*> idx_vals)
{
    AllocaInst *stored = cv::symbol_table.top()[arr_name];
    LoadInst *array = cv::ir_builder.CreateLoad(
        stored->getAllocatedType(), stored);

    Value *val_to_override;
    for (size_t i = 0; i < idx_nesting_lvl; i++)
    { 
        Value *idx_gep = cv::ir_builder.CreateGEP(
            array->getType()->getPointerElementType(), 
            array, idx_vals[i]);

        val_to_override = idx_gep;
    }

    Value *val = cv::value_stack.top();

    cv::ir_builder.CreateStore(val, val_to_override, false);
    cv::value_stack.pop();
}

void ca::array_codegen(std::vector<Value*> elems, size_t elem_count)
{
    Type *elem_type = elems[0]->getType();
    DataLayout dl = compiler::visitor::module->getDataLayout();

    Instruction *array_malloc = ca::create_array_malloc(elem_type, elem_count, dl);

    for (size_t i = 0; i < elem_count; i++)
    {
        Constant *idx = ConstantInt::get(
            Type::getInt32Ty(cv::context), i);

        Value *idx_gep = cv::ir_builder.CreateGEP(
            array_malloc->getType()->getPointerElementType(), 
            array_malloc, idx);

        cv::ir_builder.CreateStore(elems[i], idx_gep);
    }

    cv::value_stack.push(array_malloc);
}

std::any cv::Visitor::visitIndexedAccessExpr(parser::YupParser::IndexedAccessExprContext *ctx)
{
    this->visit(ctx->expr(0));
    Value *array = value_stack.top();

    this->visit(ctx->expr(1));
    Value *idx_val = value_stack.top();

    compiler::array::indexed_access_expr_codegen(array, idx_val);

    return nullptr;
}

std::any cv::Visitor::visitArr_elem_assignment(parser::YupParser::Arr_elem_assignmentContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();

    size_t idx_nesting_lvl = ctx->arr_index().size();
    std::vector<Value*> idx_vals;

    for (size_t i = 0; i < idx_nesting_lvl; i++)
    {
        this->visit(ctx->arr_index(i)->expr());
        idx_vals.push_back(value_stack.top());
    }

    ca::arr_elem_assignment_codegen(name, idx_nesting_lvl, idx_vals);

    return nullptr;
}

std::any cv::Visitor::visitArray(parser::YupParser::ArrayContext *ctx)
{
    std::vector<Value*> elems;
    size_t elem_count = ctx->expr().size();
    for (size_t i = 0; i < elem_count; i++)
    {
        this->visit(ctx->expr(i));
        Value *elem = value_stack.top();
        elems.push_back(elem);
    }

    ca::array_codegen(elems, elem_count);

    return nullptr;
}

std::any cv::Visitor::visitArray_init(parser::YupParser::Array_initContext *ctx)
{
    Type *elem_type = ct::resolve_type(ctx->type_name()->getText());

    int64_t elem_count = lexical_cast<int64_t>(ctx->V_INT()->getText());

    DataLayout dl = cv::module->getDataLayout();

    Instruction *array_malloc = ca::create_array_malloc(elem_type, elem_count, dl);
    cv::value_stack.push(array_malloc);

    return nullptr;
}

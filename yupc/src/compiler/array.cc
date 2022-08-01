#include "compiler/visitor.h"
#include "compiler/array.h"
#include "compiler/type.h"
#include "msg/errors.h"

using namespace llvm;
using namespace yupc;

namespace cv = compiler::visitor;

void compiler::array::indexed_access_expr_codegen(Value *array, Value *idxVal)
{
    Value *idx_gep = cv::ir_builder.CreateGEP(
        array->getType()->getPointerElementType(), array, idxVal);

    cv::value_stack.push(idx_gep);
}

void arr_elem_assignment_codegen(std::string arr_name, size_t idx_nesting_lvl, 
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

void array_codegen(std::vector<Value*> elems, size_t elemCount)
{
    Type *elem_type = elems[0]->getType();

    Type *i32 = Type::getInt32Ty(cv::context);

    DataLayout dl = compiler::visitor::module->getDataLayout();

    Constant *size_of_element = ConstantInt::get(
        i32, dl.getTypeAllocSize(elem_type));

    Constant *length = ConstantInt::get(i32, elemCount);
    Constant *array_size = ConstantExpr::getMul(size_of_element, length);    

    Instruction *array_malloc = CallInst::CreateMalloc(
        cv::ir_builder.GetInsertBlock(), 
        elem_type, 
        elem_type, array_size,
        nullptr, nullptr, ""); 

    cv::ir_builder.Insert(array_malloc);

    for (size_t i = 0; i < elemCount; i++)
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

    arr_elem_assignment_codegen(name, idx_nesting_lvl, idx_vals);

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

    array_codegen(elems, elem_count);

    return nullptr;
}
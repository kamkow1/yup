#include <compiler/visitor.h>
#include <compiler/array.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>
#include <msg/errors.h>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/GlobalVariable.h>

#include <cstddef>
#include <cstdint>

#include <boost/lexical_cast.hpp>

using namespace llvm;
using namespace yupc;
using namespace boost;

namespace cv = compiler::visitor;
namespace ca = compiler::array;
namespace ct = compiler::type;
namespace com_un = compiler::compilation_unit;

Instruction *ca::create_array_dyn_malloc(Type *elem_type, Value *elem_count, DataLayout dl) {
    auto *i32 = Type::getInt32Ty(*com_un::comp_units.back()->context);

    auto *size_of_element = ConstantInt::get(i32, dl.getTypeAllocSize(elem_type));

    auto *array_size = com_un::comp_units.back()->ir_builder->CreateMul(size_of_element, elem_count);

    auto *array_malloc = CallInst::CreateMalloc(
        com_un::comp_units.back()->ir_builder->GetInsertBlock(), 
        elem_type, 
        elem_type, array_size,
        nullptr, nullptr, ""); 

    com_un::comp_units.back()->ir_builder->Insert(array_malloc);

    return array_malloc;
}

Instruction *ca::create_array_const_malloc(Type *elem_type, uint64_t elem_count, DataLayout dl) {
    auto *i32 = Type::getInt32Ty(*com_un::comp_units.back()->context);

    auto *size_of_element = ConstantInt::get(i32, dl.getTypeAllocSize(elem_type));

    auto *length = ConstantInt::get(i32, elem_count);

    auto *array_size = ConstantExpr::getMul(size_of_element, length);

    auto *array_malloc = CallInst::CreateMalloc(
        com_un::comp_units.back()->ir_builder->GetInsertBlock(), 
        elem_type, 
        elem_type, array_size,
        nullptr, nullptr, ""); 

    com_un::comp_units.back()->ir_builder->Insert(array_malloc);

    return array_malloc;
}

void ca::indexed_access_expr_codegen(Value *array, Value *idxVal) {

    auto *ptr_type = array->getType()->getPointerElementType();

    auto *idx_gep = com_un::comp_units.back()->ir_builder->CreateGEP(ptr_type, array, idxVal);

    com_un::comp_units.back()->value_stack.push(idx_gep);
}

void ca::arr_elem_assignment_codegen(std::string arr_name, 
                                size_t idx_nesting_lvl, 
                                std::vector<Value*> idx_vals) {

    auto *stored = com_un::comp_units.back()->symbol_table.back()[arr_name];

    auto *alloc_type = stored->getAllocatedType();
    auto *array = com_un::comp_units.back()->ir_builder->CreateLoad(alloc_type, stored);

    Value *val_to_override;
    for (auto i = 0; i < idx_nesting_lvl; i++) { 

        auto *arr_ptr_type = array->getType()->getPointerElementType();

        auto *idx_gep = com_un::comp_units.back()->ir_builder->CreateGEP(arr_ptr_type, array, idx_vals[i]);

        val_to_override = idx_gep;
    }

    auto *val = com_un::comp_units.back()->value_stack.top();

    com_un::comp_units.back()->ir_builder->CreateStore(val, val_to_override, false);

    com_un::comp_units.back()->value_stack.pop();
}

void ca::array_codegen(std::vector<Value*> elems, size_t elem_count) {

    auto *elem_type = elems[0]->getType();
    auto dl = com_un::comp_units.back()->module->getDataLayout();

    auto *array_malloc = ca::create_array_const_malloc(elem_type, elem_count, dl);

    for (auto i = 0; i < elem_count; i++) {
        auto *idx = ConstantInt::get(Type::getInt32Ty(*com_un::comp_units.back()->context), i);

        auto *alloc_ptr_type = array_malloc->getType()->getPointerElementType();

        auto *idx_gep = com_un::comp_units.back()->ir_builder->CreateGEP(alloc_ptr_type, array_malloc, idx);

        com_un::comp_units.back()->ir_builder->CreateStore(elems[i], idx_gep);
    }

    com_un::comp_units.back()->value_stack.push(array_malloc);
}

std::any cv::Visitor::visitIndexedAccessExpr(parser::YupParser::IndexedAccessExprContext *ctx) {

    this->visit(ctx->expr(0));
    auto *array = com_un::comp_units.back()->value_stack.top();

    this->visit(ctx->expr(1));
    auto *idx_val = com_un::comp_units.back()->value_stack.top();

    ca::indexed_access_expr_codegen(array, idx_val);

    return nullptr;
}

std::any cv::Visitor::visitArr_elem_assignment(parser::YupParser::Arr_elem_assignmentContext *ctx) {

    auto name = ctx->IDENTIFIER()->getText();

    auto idx_nesting_lvl = ctx->arr_index().size();
    std::vector<Value*> idx_vals;

    for (auto i = 0; i < idx_nesting_lvl; i++) {
        this->visit(ctx->arr_index(i)->expr());

        idx_vals.push_back(com_un::comp_units.back()->value_stack.top());
    }

    ca::arr_elem_assignment_codegen(name, idx_nesting_lvl, idx_vals);

    return nullptr;
}

std::any cv::Visitor::visitArray(parser::YupParser::ArrayContext *ctx) {

    std::vector<Value*> elems;
    auto elem_count = ctx->expr().size();
    for (auto i = 0; i < elem_count; i++) {
        this->visit(ctx->expr(i));

        auto *elem = com_un::comp_units.back()->value_stack.top();

        elems.push_back(elem);
    }

    ca::array_codegen(elems, elem_count);

    return nullptr;
}

std::any cv::Visitor::visitArray_init(parser::YupParser::Array_initContext *ctx) {

    auto *elem_type = ct::resolve_type(ctx->type_name()->getText());

    this->visit(ctx->expr());
    auto *array_size = com_un::comp_units.back()->value_stack.top();

    auto dl = com_un::comp_units.back()->module->getDataLayout();

    auto *array_malloc = ca::create_array_dyn_malloc(elem_type, array_size, dl);
        
    com_un::comp_units.back()->value_stack.push(array_malloc);

    return nullptr;
}

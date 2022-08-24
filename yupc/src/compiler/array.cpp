#include <compiler/visitor.h>
#include <compiler/array.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>
#include <msg/errors.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/GlobalVariable.h>

#include <boost/lexical_cast.hpp>

#include <cstddef>
#include <string>

llvm::Instruction *yupc::create_array_dyn_malloc(llvm::Type *elem_type, llvm::Value *elem_count, llvm::DataLayout dl) 
{
    llvm::IntegerType *i32 = llvm::Type::getInt32Ty(*yupc::comp_units.back()->context);
    llvm::ConstantInt *size_of_element = llvm::ConstantInt::get(i32, dl.getTypeAllocSize(elem_type));
    llvm::Value *array_size = yupc::comp_units.back()->ir_builder->CreateMul(size_of_element, elem_count);
    llvm::BasicBlock *basic_block = yupc::comp_units.back()->ir_builder->GetInsertBlock();
    llvm::Instruction *array_malloc = llvm::CallInst::CreateMalloc(basic_block, elem_type, elem_type, 
                                                                    array_size, nullptr, nullptr, ""); 
    yupc::comp_units.back()->ir_builder->Insert(array_malloc);
    return array_malloc;
}

llvm::Instruction *yupc::create_array_const_malloc(llvm::Type *elem_type, uint64_t elem_count, llvm::DataLayout dl) 
{
    llvm::IntegerType *i32 = llvm::Type::getInt32Ty(*yupc::comp_units.back()->context);
    llvm::ConstantInt *size_of_element = llvm::ConstantInt::get(i32, dl.getTypeAllocSize(elem_type));
    llvm::ConstantInt *length = llvm::ConstantInt::get(i32, elem_count);
    llvm::Constant *array_size = llvm::ConstantExpr::getMul(size_of_element, length);
    llvm::BasicBlock *basic_block = yupc::comp_units.back()->ir_builder->GetInsertBlock();
    llvm::Instruction *array_malloc = llvm::CallInst::CreateMalloc(basic_block, elem_type, elem_type, 
                                                                    array_size, nullptr, nullptr, ""); 
    yupc::comp_units.back()->ir_builder->Insert(array_malloc);
    return array_malloc;
}

void yupc::indexed_access_expr_codegen(llvm::Value *array, llvm::Value *idxVal) 
{
    llvm::Type *ptr_type = array->getType()->getPointerTo();
    llvm::AllocaInst *arr_ptr = yupc::comp_units.back()->ir_builder->CreateAlloca(array->getType()->getPointerTo());
    llvm::Value *idx_gep = yupc::comp_units.back()->ir_builder->CreateGEP(ptr_type, arr_ptr, idxVal);
    yupc::comp_units.back()->value_stack.push(idx_gep);
}

void yupc::arr_elem_assignment_codegen(std::string arr_name, size_t idx_nesting_lvl, std::vector<llvm::Value*> idx_vals) 
{
    llvm::AllocaInst *stored = yupc::comp_units.back()->symbol_table.back()[arr_name];
    llvm::Type *alloc_type = stored->getAllocatedType();
    llvm::LoadInst *array = yupc::comp_units.back()->ir_builder->CreateLoad(alloc_type, stored);

    llvm::Value *val_to_override;
    for (size_t i = 0; i < idx_nesting_lvl; i++) 
    { 
        llvm::Type *arr_ptr_type = array->getType()->getPointerTo();
        llvm::AllocaInst *arr_ptr = yupc::comp_units.back()->ir_builder->CreateAlloca(array->getType()->getPointerTo());
        llvm::Value *idx_gep = yupc::comp_units.back()->ir_builder->CreateGEP(arr_ptr_type, arr_ptr, idx_vals[i]);
        val_to_override = idx_gep;
    }

    llvm::Value *val = yupc::comp_units.back()->value_stack.top();
    yupc::comp_units.back()->ir_builder->CreateStore(val, val_to_override, false);
    yupc::comp_units.back()->value_stack.pop();
}

void yupc::array_codegen(std::vector<llvm::Value*> elems, size_t elem_count) 
{
    llvm::Type *elem_type = elems[0]->getType();
    llvm::DataLayout dl = yupc::comp_units.back()->module->getDataLayout();
    llvm::Instruction *array_malloc = yupc::create_array_const_malloc(elem_type, elem_count, dl);

    for (size_t i = 0; i < elem_count; i++) 
    {
        llvm::ConstantInt *idx = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*yupc::comp_units.back()->context), i);
        llvm::Type *alloc_ptr_type = array_malloc->getType()->getPointerTo();
        llvm::Value *idx_gep = yupc::comp_units.back()->ir_builder->CreateGEP(alloc_ptr_type, array_malloc, idx);
        yupc::comp_units.back()->ir_builder->CreateStore(elems[i], idx_gep);
    }

    yupc::comp_units.back()->value_stack.push(array_malloc);
}

std::any yupc::Visitor::visitIndexedAccessExpr(yupc::YupParser::IndexedAccessExprContext *ctx) 
{
    this->visit(ctx->expr(0));
    llvm::Value *array = yupc::comp_units.back()->value_stack.top();
    for (size_t i = 1; i < ctx->expr().size(); i++) 
    {
        this->visit(ctx->expr(i));
        llvm::Value *idx_val = yupc::comp_units.back()->value_stack.top();
        yupc::indexed_access_expr_codegen(array, idx_val);
    }

    return nullptr;
}

std::any yupc::Visitor::visitArr_elem_assignment(yupc::YupParser::Arr_elem_assignmentContext *ctx) 
{
    std::string name = ctx->IDENTIFIER()->getText();

    size_t idx_nesting_lvl = ctx->arr_index().size();
    std::vector<llvm::Value*> idx_vals;
    for (size_t i = 0; i < idx_nesting_lvl; i++) 
    {
        this->visit(ctx->arr_index(i)->expr());
        idx_vals.push_back(yupc::comp_units.back()->value_stack.top());
    }

    yupc::arr_elem_assignment_codegen(name, idx_nesting_lvl, idx_vals);
    return nullptr;
}

std::any yupc::Visitor::visitArray(yupc::YupParser::ArrayContext *ctx) 
{
    std::vector<llvm::Value*> elems;
    size_t elem_count = ctx->expr().size();
    for (size_t i = 0; i < elem_count; i++) 
    {
        this->visit(ctx->expr(i));
        llvm::Value *elem = yupc::comp_units.back()->value_stack.top();
        elems.push_back(elem);
    }

    yupc::array_codegen(elems, elem_count);
    return nullptr;
}

std::any yupc::Visitor::visitArray_init(yupc::YupParser::Array_initContext *ctx) 
{

    llvm::Type *elem_type = yupc::resolve_type(ctx->type_name()->getText(), 
                                            yupc::comp_units.back()->module->getContext());
    this->visit(ctx->expr());
    llvm::Value *array_size = yupc::comp_units.back()->value_stack.top();
    llvm::DataLayout dl = yupc::comp_units.back()->module->getDataLayout();
    llvm::Instruction *array_malloc = yupc::create_array_dyn_malloc(elem_type, array_size, dl);
    yupc::comp_units.back()->value_stack.push(array_malloc);
    return nullptr;
}

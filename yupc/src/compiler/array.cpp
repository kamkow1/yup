#include <compiler/visitor.h>
#include <compiler/array.h>
#include <compiler/type.h>
#include <compiler/compilation_unit.h>
#include <llvm/Support/Casting.h>
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

#include <cstddef>
#include <string>

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

    llvm::Value *ptr;
    for (size_t i = 0; i < idx_nesting_lvl; i++) 
    { 
        llvm::Value *gep = yupc::comp_units.back()->ir_builder->CreateInBoundsGEP(stored->getAllocatedType(), stored, idx_vals[i]);
        ptr = gep;
    }

    llvm::Value *val = yupc::comp_units.back()->value_stack.top();
    ptr->mutateType(stored->getAllocatedType()->getArrayElementType()->getPointerTo());
    yupc::comp_units.back()->ir_builder->CreateStore(val, ptr);
}

void yupc::array_codegen(std::vector<llvm::Constant*> elems, size_t elem_count) 
{
    llvm::Type *elem_type = elems[0]->getType();
    llvm::ArrayType *array_type = llvm::ArrayType::get(elem_type, elems.size());
    llvm::Constant *const_array = llvm::ConstantArray::get(array_type, elems);

    yupc::comp_units.back()->value_stack.push(const_array);
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
        yupc::comp_units.back()->value_stack.pop();
    }

    yupc::comp_units.back()->value_stack.pop();

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
    std::vector<llvm::Constant*> elems;
    size_t elem_count = ctx->expr().size();
    for (size_t i = 0; i < elem_count; i++) 
    {
        this->visit(ctx->expr(i));
        llvm::Constant *elem = llvm::cast<llvm::Constant>(yupc::comp_units.back()->value_stack.top());
        elems.push_back(elem);
        yupc::comp_units.back()->value_stack.pop();
    }

    yupc::array_codegen(elems, elem_count);
    return nullptr;
}

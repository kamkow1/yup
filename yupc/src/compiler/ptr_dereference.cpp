#include <compiler/visitor.h>
#include <compiler/ptr_dereference.h>
#include <compiler/compilation_unit.h>

#include <msg/errors.h>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

#include <any>

using namespace llvm;
using namespace yupc;

namespace cv = compiler::visitor;
namespace ptr_deref = compiler::ptr_dereference;
namespace com_un = compiler::compilation_unit;

void ptr_deref::ptr_deref_codegen(Value *value, std::string text) {

    if (!value->getType()->isPointerTy()) {
        msg::errors::log_compiler_err("cannot dereference a non pointer type expression", text);
        exit(1);
    }

    auto *load = com_un::comp_units.back()->ir_builder->CreateLoad(value->getType()->getNonOpaquePointerElementType(), value);
    com_un::comp_units.back()->value_stack.push(load);
}

std::any cv::Visitor::visitPtr_dereference(parser::YupParser::Ptr_dereferenceContext *ctx) {

    this->visit(ctx->expr());
    auto *value = com_un::comp_units.back()->value_stack.top();

    ptr_deref::ptr_deref_codegen(value, ctx->getText());

    return nullptr;
}

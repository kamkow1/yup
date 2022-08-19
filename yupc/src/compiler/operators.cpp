#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/operators.h>

#include <llvm/IR/Constants.h>
#include <parser/YupParser.h>

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <any>
#include <vector>
#include <map>
#include <string>
#include <stdarg.h>

using namespace yupc;
using namespace llvm;

namespace cv = compiler::visitor;
namespace com_un = compiler::compilation_unit;
namespace co = compiler::operators;

Value *size_oper(std::vector<co::OperArg*> args) {
    auto *base_sz = args[0];
    auto *size = ConstantExpr::getSizeOf(base_sz->oper_type);

    return size;
}


std::map<std::string, Value *(*)(std::vector<co::OperArg*>)> co::opers {
    { "size", &size_oper }
};

std::any cv::Visitor::visitOperator(parser::YupParser::OperatorContext *ctx) {

    auto oper_name = ctx->IDENTIFIER()->getText();

    std::vector<co::OperArg*> args;
    for (auto *aa : ctx->expr()) {

        auto *oa_un = new co::OperArg;

        if (dynamic_cast<parser::YupParser::TypeAnnotExprContext*>(aa) != nullptr) {
            this->visit(aa);

            oa_un->oper_type = com_un::comp_units.back()->type_stack.top();
            args.push_back(oa_un);
        } else {
            this->visit(aa);

            oa_un->oper_value = com_un::comp_units.back()->value_stack.top();
            args.push_back(oa_un);
        }
    }

    auto *oper = co::opers[oper_name];
    auto *result = oper(args);

    com_un::comp_units.back()->value_stack.push(result);

    return nullptr;
}

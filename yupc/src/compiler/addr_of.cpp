#include <compiler/visitor.h>
#include <compiler/addr_of.h>
#include <compiler/compilation_unit.h>

void yupc::addr_of_expr_codegen(std::string id) 
{
    llvm::Value *val = yupc::comp_units.back()->symbol_table.back()[id];
    yupc::comp_units.back()->value_stack.push(val);
}

std::any yupc::Visitor::visitAddr_of(yupc::YupParser::Addr_ofContext *ctx) 
{
    std::string text = ctx->IDENTIFIER()->getText();
    yupc::addr_of_expr_codegen(text);   
    return nullptr;
}

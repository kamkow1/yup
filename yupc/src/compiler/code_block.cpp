#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/code_block.h>

#include <parser/YupParser.h>

#include <llvm/IR/Instructions.h>

#include <string>

void yupc::create_new_scope()
{
    std::map<std::string, llvm::AllocaInst*> symbol_map;
    yupc::comp_units.back()->symbol_table.push_back(symbol_map);
}

void yupc::drop_scope()
{
    yupc::comp_units.back()->symbol_table.pop_back();
}

std::any yupc::Visitor::visitCode_block(yupc::YupParser::Code_blockContext *ctx) 
{
    yupc::create_new_scope();

    for (yupc::YupParser::StatementContext *statement : ctx->statement()) 
    {
        this->visit(statement);
    }

    yupc::drop_scope();
    return nullptr;
}

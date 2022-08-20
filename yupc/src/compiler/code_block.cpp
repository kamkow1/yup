#include <compiler/visitor.h>
#include <parser/YupParser.h>

std::any yupc::Visitor::visitCode_block(yupc::YupParser::Code_blockContext *ctx) 
{

    for (yupc::YupParser::StatementContext *statement : ctx->statement()) 
    {
        this->visit(statement);
    }

    return nullptr;
}

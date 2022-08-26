#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/CodeBlock.h"

#include "Parser/YupParser.h"

#include "llvm/IR/Instructions.h"

#include <string>

void yupc::CreateNewScope()
{
    std::map<std::string, llvm::AllocaInst*> newSymbolMap;
    yupc::CompilationUnits.back()->SymbolTable.push_back(newSymbolMap);
}

void yupc::DropScope()
{
    yupc::CompilationUnits.back()->SymbolTable.pop_back();
}

std::any yupc::Visitor::visitCodeBlock(yupc::YupParser::CodeBlockContext *ctx) 
{
    yupc::CreateNewScope();

    for (yupc::YupParser::StatementContext *statement : ctx->statement()) 
    {
        this->visit(statement);
    }

    yupc::DropScope();
    return nullptr;
}

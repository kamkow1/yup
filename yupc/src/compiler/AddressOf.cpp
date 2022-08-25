#include "compiler/Visitor.h"
#include "compiler/AddressOf.h"
#include "compiler/CompilationUnit.h"

void yupc::AddressOfCodegen(std::string id) 
{
    llvm::Value *val = yupc::CompilationUnits.back()->SymbolTable.back()[id];
    yupc::CompilationUnits.back()->ValueStack.push(val);
}

std::any yupc::Visitor::visitAddressOf(yupc::YupParser::AddressOfContext *ctx) 
{
    std::string text = ctx->Identifier()->getText();
    yupc::AddressOfCodegen(text);   
    return nullptr;
}

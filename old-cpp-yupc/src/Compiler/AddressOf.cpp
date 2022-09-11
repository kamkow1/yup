#include "Compiler/Visitor.h"
#include "Compiler/AddressOf.h"
#include "Compiler/CompilationUnit.h"

#include "llvm/IR/Value.h"

llvm::Value *yupc::AddressOfCodegen(std::string id) 
{
    return yupc::CompilationUnits.back()->SymbolTable.back()[id]->ValuePtr;
}

std::any yupc::Visitor::visitAddressOf(yupc::YupParser::AddressOfContext *ctx) 
{
    std::string text = ctx->Identifier()->getText();
    llvm::Value *val = yupc::AddressOfCodegen(text);   
    yupc::CompilationUnits.back()->ValueStack.push(val);
    return nullptr;
}

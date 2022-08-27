#include "Compiler/Visitor.h"
#include "Compiler/Type.h"
#include "Compiler/Variable.h"
#include "Compiler/CompilationUnit.h"
#include "Logger.h"

#include "llvm/Support/TypeName.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Alignment.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Metadata.h"

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

static std::map<std::string, yupc::VariableInfo*> Variables;

llvm::AllocaInst *yupc::FindLocalVariable(std::string name, size_t i,
    std::vector<std::map<std::string, llvm::AllocaInst*>> &symbolTable, 
    size_t line, size_t pos, std::string text)
{
    if (symbolTable[i].contains(name))
    {
        return symbolTable[i][name];
    }
    else if (i > 0)
    {
        return yupc::FindLocalVariable(name, --i, symbolTable, line, pos, text);
    }
    else
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "unknown variable " + name, text, 
                                            yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }
}

llvm::LoadInst *yupc::IdentifierCodegen(std::string id, bool isGlobal, 
                size_t line, size_t pos, std::string text) 
{
    if (isGlobal)
    {
        llvm::GlobalVariable *globalVariable = yupc::CompilationUnits.back()->GlobalVariables[id];
        llvm::Type *type = globalVariable->getValueType();
        llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(type, globalVariable);
        return load;
    }
    else
    {
        llvm::AllocaInst *variable = yupc::FindLocalVariable(id, yupc::CompilationUnits.back()->SymbolTable.size() - 1,
                                                            yupc::CompilationUnits.back()->SymbolTable, line, pos, text);

        llvm::Type *type = variable->getAllocatedType();
        llvm::LoadInst *load = yupc::CompilationUnits.back()->IRBuilder->CreateLoad(type, variable);
        return load;
    }
}

void yupc::AssignmentCodegen(std::string name, llvm::Value *val, 
                        size_t line, size_t pos, std::string text) 
{
    yupc::VariableInfo *var = Variables[name];

    if (var->IsConstant) 
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "cannot reassign a constant \"" + name + "\"", 
                                            text, yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    if (var->IsReference) 
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "cannot make a reference point to another variable", 
                                            text, yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    bool isLocal  = yupc::CompilationUnits.back()->SymbolTable.back().contains(name);
    bool isGlobal = yupc::CompilationUnits.back()->GlobalVariables.contains(name);

    if (isLocal) 
    {
        llvm::AllocaInst *stored = yupc::CompilationUnits.back()->SymbolTable.back()[name];
        if (!yupc::CheckValueType(val, stored))
        {
            std::string valTypeString = yupc::TypeToString(val->getType());
            std::string storedTypeString = yupc::TypeToString(stored->getType());

            yupc::GlobalLogger.LogCompilerError(line, pos, "cannot assign type " + valTypeString + " to " 
                                    + storedTypeString, text, yupc::CompilationUnits.back()->SourceFile);
            exit(1);
        }

        yupc::CompilationUnits.back()->IRBuilder->CreateStore(val, stored, false);
        yupc::CompilationUnits.back()->ValueStack.pop();
    } 
    else if (isGlobal) 
    {
        llvm::GlobalVariable *gv = yupc::CompilationUnits.back()->GlobalVariables[name];
        if (!yupc::CheckValueType(val, gv))
        {
            std::string gvTypeString  = yupc::TypeToString(gv->getValueType());
            std::string valTypeString = yupc::TypeToString(val->getType());

            yupc::GlobalLogger.LogCompilerError(line, pos, "cannot assign type " + valTypeString + " to " 
                                        + gvTypeString, text, yupc::CompilationUnits.back()->SourceFile);
            exit(1);
        }

        yupc::CompilationUnits.back()->IRBuilder->CreateStore(val, gv, false);
        yupc::CompilationUnits.back()->ValueStack.pop();
    }
    else 
    {
        yupc::GlobalLogger.LogCompilerError(line, pos, "cannot reassign \"" + name + "\" because it doesn't exist", 
                                            text, yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }
}

void yupc::VarDeclareCodegen(std::string name, llvm::Type *resolvedType, bool isConst, 
                            bool isGlobal, bool isPublic, bool isReference, llvm::Value *val) 
{
    if (isGlobal) 
    {
        llvm::GlobalValue::LinkageTypes lt = isPublic ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::PrivateLinkage;
        llvm::GlobalVariable *gv = new llvm::GlobalVariable(*yupc::CompilationUnits.back()->Module, resolvedType,
                                                            isConst, lt, 0, name);

        gv->setDSOLocal(true);
        gv->setInitializer((llvm::Constant*) val);

        yupc::CompilationUnits.back()->GlobalVariables[name] = gv;
        yupc::CompilationUnits.back()->ValueStack.push(gv);

        Variables[name] = new yupc::VariableInfo{name, isConst, isReference};
    } 
    else 
    {
        llvm::AllocaInst *ptr = yupc::CompilationUnits.back()->IRBuilder->CreateAlloca(resolvedType, 0, "");

        if (val != nullptr) 
        {
            yupc::CompilationUnits.back()->IRBuilder->CreateStore(val, ptr, false);
        }

        yupc::CompilationUnits.back()->SymbolTable.back()[name] = ptr;
        yupc::CompilationUnits.back()->ValueStack.push(ptr);

        Variables[name] = new yupc::VariableInfo{name, isConst, isReference};
    }
}

std::any yupc::Visitor::visitVariableDeclare(yupc::YupParser::VariableDeclareContext *ctx) 
{
    std::string name = ctx->Identifier()->getText();
    
    bool isGlob  = ctx->KeywordGlobal()                    != nullptr;
    bool isConst = ctx->declarationType()->KeywordConst()  != nullptr;
    bool isPub   = ctx->KeywordExport()                    != nullptr;
    bool isRef   = ctx->KeywordRef()                       != nullptr;

    if (isRef && ctx->variableValue() == nullptr) 
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                            "cannot declare a reference that doesn't point to a variable", 
                                            ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    bool globalsContain = yupc::CompilationUnits.back()->GlobalVariables.contains(name);

    if (isGlob && globalsContain) 
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                    "global variable \"" + name + ctx->typeAnnotation()->getText() + "\" already exists", 
                    ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    bool localsContains = false;
    if (yupc::CompilationUnits.back()->SymbolTable.size() != 0) 
    {
        yupc::CompilationUnits.back()->SymbolTable.back().contains(name);
    }

    if (!isGlob && localsContains) 
    {
        yupc::GlobalLogger.LogCompilerError(ctx->start->getLine(), ctx->start->getCharPositionInLine(), 
                                            "variable \"" + name + ctx->typeAnnotation()->getText() 
                                            + "\" has already been declared in this scope", 
                                            ctx->getText(), yupc::CompilationUnits.back()->SourceFile);
        exit(1);
    }

    this->visit(ctx->typeAnnotation());
    llvm::Type *resolvedType = yupc::CompilationUnits.back()->TypeStack.top();

    llvm::Value *val = ctx->variableValue() != nullptr 
        ? [&]() {this->visit(ctx->variableValue()->expression()); 
                return yupc::CompilationUnits.back()->ValueStack.top();}()
        : nullptr;

    if (val != nullptr)
    {
        /*if (resolved_type != val->getType())
        {
            std::string rt_str;
            llvm::raw_string_ostream rt_rso(rt_str);
            resolved_type->print(rt_rso);

            std::string vt_str;
            llvm::raw_string_ostream vt_rso(vt_str);
            val->getType()->print(vt_rso);

            yupc::log_compiler_err("mismatch of types " + rt_str + " and " + vt_str, ctx->getText());
            exit(1);
        }*/
    }

    yupc::VarDeclareCodegen(name, resolvedType, isConst, isGlob, isPub, isRef, val);

    return nullptr;
}

std::any yupc::Visitor::visitAssignment(yupc::YupParser::AssignmentContext *ctx) 
{
    std::string name = ctx->Identifier()->getText();

    this->visit(ctx->variableValue()->expression());
    llvm::Value *val = yupc::CompilationUnits.back()->ValueStack.top();

    yupc::AssignmentCodegen(name, val, ctx->start->getLine(), 
            ctx->start->getCharPositionInLine(), ctx->getText());
    
    return nullptr;
}

std::any yupc::Visitor::visitIdentifierExpression(yupc::YupParser::IdentifierExpressionContext *ctx) 
{
    std::string name = ctx->Identifier()->getText();

    bool isGlob = yupc::CompilationUnits.back()->GlobalVariables.contains(name);

    llvm::LoadInst *load = yupc::IdentifierCodegen(name, isGlob, ctx->start->getLine(), 
                                    ctx->start->getCharPositionInLine(), ctx->getText());
    yupc::CompilationUnits.back()->ValueStack.push(load);
    return nullptr;
}

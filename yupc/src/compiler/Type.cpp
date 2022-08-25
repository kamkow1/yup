#include "Logger.h"
#include "compiler/Visitor.h"
#include "compiler/Type.h"
#include "compiler/CompilationUnit.h"
#include "Lexer.h"

#include "parser/YupParser.h"
#include "lexer/YupLexer.h"

#include "utils.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <any>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <map>
#include <algorithm>

yupc::AliasType::AliasType(std::string _typeName,
                        std::string _destination,
                        bool _isPublic)
:   TypeName(_typeName),
    Destination(_destination),
    IsPublic(_isPublic) { }

llvm::Type *yupc::ResolvePointerType(llvm::Type *base) 
{
    return llvm::PointerType::get(base, 0);
}

std::map<std::string, yupc::BuiltInTypes> yupc::BuiltInLLVMTypes
{
    {"i32", yupc::LLVM_I32_TYPE},
    {"i64", yupc::LLVM_I64_TYPE},
    {"float", yupc::LLVM_FLOAT_TYPE},
    {"void", yupc::LLVM_VOID_TYPE},
    {"i8", yupc::LLVM_I8_TYPE}
};

std::string yupc::TypeToString(llvm::Type *type)
{
    std::string str;
    llvm::raw_string_ostream rso(str);
    type->print(rso);
    return str;
}

llvm::Type *yupc::GetBuiltInLLVMType(std::string typeName, llvm::LLVMContext &contextRef)
{
    switch (yupc::BuiltInLLVMTypes[typeName]) 
    {
        case yupc::LLVM_I32_TYPE:
            return llvm::Type::getInt32Ty(contextRef);
        case yupc::LLVM_I64_TYPE:
            return llvm::Type::getInt64Ty(contextRef);
        case yupc::LLVM_I8_TYPE:
            return llvm::Type::getInt8Ty(contextRef);
        case yupc::LLVM_FLOAT_TYPE:
            return llvm::Type::getFloatTy(contextRef);
        case yupc::LLVM_VOID_TYPE:
            return llvm::Type::getVoidTy(contextRef);
    }

    return nullptr;
}

llvm::Type* yupc::ResolveType(std::string typeName, llvm::LLVMContext &contextRef) 
{
    for (size_t i = 0; i < yupc::CompilationUnits.back()->TypeAliases.size(); i++) 
    {
        yupc::AliasType *aliasType = yupc::CompilationUnits.back()->TypeAliases[i];

        if (aliasType->IsPublic) 
        {

            if (aliasType->TypeName == typeName) {
                typeName = yupc::CompilationUnits.back()->TypeAliases[i]->Destination;
            }
        }
    }

    if (yupc::BuiltInLLVMTypes.contains(typeName))
    {
        return yupc::GetBuiltInLLVMType(typeName, contextRef);
    }
    else
    {
        std::string baseString = typeName;
        baseString.erase(std::remove(baseString.begin(), baseString.end(), '*'), baseString.end());

        llvm::Type *base = yupc::ResolveType(baseString, contextRef);

        std::string suffixes = typeName;
        yupc::string_remove_all(baseString, suffixes);
            
        for (size_t i = 0; i < suffixes.size(); i++) 
        {
            auto c = suffixes[i];
            if (c == '*') 
            {
                base = yupc::ResolvePointerType(base);
            }
        }

        return base;
    }
}

llvm::Type *yupc::ResolveFixedArrayType(llvm::Type *base, uint64_t size) 
{
    return llvm::ArrayType::get(base, size);        
}

bool yupc::CheckValueType(llvm::Value *val1, llvm::Value *val2) 
{
    if (val1->getType()->isPointerTy())
    {
        return val1->getType()->getTypeID() == val2->getType()->getTypeID();
    }
    else
    {
        return val1->getType()->getPointerTo()->getTypeID() == val2->getType()->getTypeID();
    }
}

std::any yupc::Visitor::visitTypeNameExpression(yupc::YupParser::TypeNameExpressionContext *ctx) {return this->visit(ctx->typeName());}
std::any yupc::Visitor::visitTypeExpression(yupc::YupParser::TypeExpressionContext *ctx)         {return this->visit(ctx->typeNameExpression());}
std::any yupc::Visitor::visitTypeAnnotation(yupc::YupParser::TypeAnnotationContext *ctx)         {return this->visit(ctx->typeName());}

std::any yupc::Visitor::visitTypeDeclaration(yupc::YupParser::TypeDeclarationContext *ctx) 
{
    if (ctx->typeDefinition()->typeAlias() != nullptr) 
    {
        std::string aliasName = ctx->Identifier()->getText();
        std::string oldName = ctx->typeDefinition()->typeAlias()->typeName()->getText();

        bool isPublic = ctx->KeywordPublic() != nullptr;
        yupc::AliasType *type = new yupc::AliasType(aliasName, oldName, isPublic);
        yupc::CompilationUnits.back()->TypeAliases.push_back(type);
    }

    return nullptr;
}

std::any yupc::Visitor::visitTypeCastExpression(yupc::YupParser::TypeCastExpressionContext *ctx) 
{
    this->visit(ctx->expression(0));
    llvm::Type *destType = yupc::CompilationUnits.back()->TypeStack.top();

    this->visit(ctx->expression(1));
    llvm::Value *val = yupc::CompilationUnits.back()->ValueStack.top();

    llvm::Value *cast;
    if (val->getType()->isIntegerTy() && destType->isIntegerTy())
    {
        cast = yupc::CompilationUnits.back()->IRBuilder->CreateIntCast(val, destType, true);
    }
    else if (val->getType()->isIntegerTy() && destType->isPointerTy())
    {
        cast = yupc::CompilationUnits.back()->IRBuilder->CreateIntToPtr(val, destType);
    }
    else if (val->getType()->isPointerTy() && destType->isPointerTy())
    {
        cast = yupc::CompilationUnits.back()->IRBuilder->CreatePointerCast(val, destType);
    }
    else
    {
        cast = yupc::CompilationUnits.back()->IRBuilder->CreateBitCast(val, destType);
    }

    yupc::CompilationUnits.back()->ValueStack.pop();
    yupc::CompilationUnits.back()->TypeStack.pop();

    CompilationUnits.back()->ValueStack.push(cast);

    return nullptr;
}

std::any yupc::Visitor::visitTypeName(yupc::YupParser::TypeNameContext *ctx)
{
    std::string base = ctx->Identifier()->getText();

    for (size_t i = 0; i < yupc::CompilationUnits.back()->TypeAliases.size(); i++) 
    {
        yupc::AliasType *at = yupc::CompilationUnits.back()->TypeAliases[i];
        if (at->TypeName == base) {base = at->Destination;}
    }    

    llvm::Type *typeBase = yupc::ResolveType(base, yupc::CompilationUnits.back()->Module->getContext());

    size_t extLength = ctx->typeExtension().size();
    for (size_t i = 0; i < extLength; i++) 
    {
        yupc::YupParser::TypeExtensionContext *ext = ctx->typeExtension(i);

        if (ext->SymbolAsterisk() != nullptr)
        {
            base += "*";
            typeBase = yupc::ResolveType(base, yupc::CompilationUnits.back()->Module->getContext());
        }

        if (ext->arrayTypeExtension() != nullptr)
        {
            if (dynamic_cast<yupc::YupParser::ConstantExpressionContext*>(ext->arrayTypeExtension()->expression())) 
            {
                uint32_t elemCount = yupc::string_to_uint64_t(ext->arrayTypeExtension()->expression()->getText());
                typeBase = yupc::ResolveFixedArrayType(typeBase, elemCount);
            }
        }
    }

    CompilationUnits.back()->TypeStack.push(typeBase);

    return nullptr;
}

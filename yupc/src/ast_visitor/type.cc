#include <visitor.h>
#include <compiler/type.h>
#include <messaging/errors.h>
#include <util.h>

TypeAnnotation::TypeAnnotation(std::string tn, uint64_t al)
{
    this->typeName = tn;
    this->arrayLen = al;
}

llvm::Type* matchBasicType(std::string typeName)
{
    if (typeName == "i32")
    {
        return llvm::Type::getInt32Ty(codegenCtx);
    } 
    else if (typeName == "i64")
    {
        return llvm::Type::getInt64Ty(codegenCtx);
    }
    else if (typeName == "float")
    {
        return llvm::Type::getFloatTy(codegenCtx);
    }
    else if (typeName == "bool")
    {
        return llvm::Type::getInt8Ty(codegenCtx);
    }
    else if (typeName == "void")
    {
        return llvm::Type::getVoidTy(codegenCtx);
    }
    else if (typeName == "char")
    {
        return llvm::Type::getInt8Ty(codegenCtx);
    }
    else
    {
        std::string errorMessage = "couldn't match type \"" + typeName + "\"";
        logCompilerError(errorMessage);
        exit(1);
    }

    // switch (strToInt(typeName.c_str())) {
        // case strToInt("i32"): return llvm::Type::getInt32Ty(codegenCtx);
        // case strToInt("i64"): return llvm::Type::getInt64Ty(codegenCtx);
        // case strToInt("float"): return llvm::Type::getFloatTy(codegenCtx);
        // case strToInt("bool"): return llvm::Type::getInt8Ty(codegenCtx);
        // case strToInt("void"): return llvm::Type::getVoidTy(codegenCtx);
        // case strToInt("char"): return llvm::Type::getInt8Ty(codegenCtx);
        // case strToInt("string"): return llvm::Type::getArrayElementType();
           // TODO: string type
    // }
}

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    return name;
}
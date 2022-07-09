#pragma once

inline constexpr unsigned int strToInt(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (strToInt(str, h+1) * 33) ^ str[h];
}

inline llvm::Type* matchType(std::string typeName)
{
    switch (strToInt(typeName.c_str())) {
        case strToInt("i32"): return llvm::Type::getInt32Ty(codegenCtx);
        case strToInt("i64"): return llvm::Type::getInt32Ty(codegenCtx);
        case strToInt("flt"): return llvm::Type::getFloatTy(codegenCtx);
        case strToInt("bool"): return llvm::Type::getInt8Ty(codegenCtx);
        case strToInt("void"): return llvm::Type::getVoidTy(codegenCtx);
            // TODO: string type and void type
    }

    fprintf(stderr, "%s%s\n", "couldn't match type ", typeName.c_str());
    exit(1);
}
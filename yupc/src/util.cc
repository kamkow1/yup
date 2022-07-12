#include <util.h>

constexpr unsigned int strToInt(const char* str, int h)
{
    return !str[h] ? 5381 : (strToInt(str, h+1) * 33) ^ str[h];
}

llvm::Type* matchBasicType(std::string typeName)
{
    switch (strToInt(typeName.c_str())) {
        case strToInt("i32"): return llvm::Type::getInt32Ty(codegenCtx);
        case strToInt("i64"): return llvm::Type::getInt64Ty(codegenCtx);
        case strToInt("float"): return llvm::Type::getFloatTy(codegenCtx);
        case strToInt("bool"): return llvm::Type::getInt8Ty(codegenCtx);
        case strToInt("void"): return llvm::Type::getVoidTy(codegenCtx);
        case strToInt("char"): return llvm::Type::getInt8Ty(codegenCtx);
        //case strToInt("string"): return llvm::Type::getArrayElementType();
            // TODO: string type
    }

    std::string errorMessage = "couldn't match type \"" + typeName + "\"";
    logCompilerError(errorMessage);

    exit(1);
}

std::string fileToString(const std::string& path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        std::string errorMessage = "unable to open file \"" + path + "\"";
        logInputError(errorMessage);
        exit(1);
    }

    auto beg = (std::istreambuf_iterator<char>(input_file));
    auto end = std::istreambuf_iterator<char>();
    return std::string(beg, end);
}

std::string dirnameOf(const std::string& fname)
{
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos)
           ? ""
           : fname.substr(0, pos);
}

std::string getIRFileName(std::string path)
{
    namespace fs = std::filesystem;

    std::string base = path.substr(path.find_last_of("/\\") + 1);
    std::string directory = dirnameOf(path);

    std::string::size_type const p(base.find_last_of('.'));
    std::string noExt = base.substr(0, p);

    fs::path fullName = directory + "/" + noExt + ".ll";
    return fullName;
}

std::string getReadableTypeName(std::string typeName)
{
    // match type string
    switch (strToInt(typeName.c_str()))
    {
    case strToInt("i8"): return "bool";
    
    default: return typeName;
        // TODO: add more cases
    }

    std::string errorMessage 
        = "couldn't \"pretty-print\" type name \"" 
        + typeName + "\"";

    exit(1);
}
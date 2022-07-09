#pragma once
#include <map>
#include <filesystem>

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

inline std::string fileToString(const std::string& path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        std::cerr << "ERROR: Could not open the file - '"
                  << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto beg = (std::istreambuf_iterator<char>(input_file));
    auto end = std::istreambuf_iterator<char>();
    return std::string(beg, end);
}

inline std::string dirnameOf(const std::string& fname)
{
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos)
           ? ""
           : fname.substr(0, pos);
}

inline std::string getIRFileName(std::string path)
{
    namespace fs = std::filesystem;

    std::string base = path.substr(path.find_last_of("/\\") + 1);
    std::string directory = dirnameOf(path);

    std::string::size_type const p(base.find_last_of('.'));
    std::string noExt = base.substr(0, p);

    fs::path fullName = directory + "/" + noExt + ".ll";
    return fullName;
}
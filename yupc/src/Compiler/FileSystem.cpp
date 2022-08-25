#include "Compiler/FileSystem.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Compiler.h"
#include "Logger.h"

#include "llvm/Support//FileSystem.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

std::string yupc::GlobalBuildDirPath;

std::string yupc::InitializeBuildDir(std::string dirBase) 
{

    fs::path b(dirBase);
    fs::path bd("build");
    fs::path path = b / bd;

    if (!fs::is_directory(path.string()) || !fs::exists(path.string())) 
    {
        fs::create_directory(path.string());
    }

    return path.string();
}

std::string yupc::InitializeBinDir() 
{
    fs::path bin(yupc::GlobalBuildDirPath);
    fs::path dir("bin");

    fs::path binDir = bin / dir;
    fs::create_directory(binDir.string());

    fs::path p(yupc::GlobalCompilerOptions.BinaryName + ".bc");
    fs::path bicodeFile = binDir / p;

    return bicodeFile;
}

void yupc::DumpModuleToIrFile(llvm::Module *module, std::string moduleName) 
{
    std::error_code ec;
    llvm::raw_fd_ostream os(moduleName, ec, llvm::sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    std::string info = "module " + yupc::CompilationUnits.back()->ModuleName;
    yupc::GlobalLogger.LogCompilerInfo(info);
}

std::string yupc::ReadFileToString(const std::string& path)
{
    std::ifstream inputFile(path);
    if (!inputFile.is_open())
    {
        yupc::GlobalLogger.LogCompilerInputError("unable to open file \"" + path + "\"");
        exit(1);
    }

    auto beg = std::istreambuf_iterator<char>(inputFile);
    auto end = std::istreambuf_iterator<char>();
    return std::string(beg, end);
}

std::string yupc::GetDirectoryName(const std::string& fname)
{
    auto pos = fname.find_last_of("\\/");
    return std::string::npos == pos ? "" : fname.substr(0, pos);
}

std::string yupc::CreateIrFileName(std::string path)
{
    std::string base = path.substr(path.find_last_of("/\\") + 1);
    std::string directory = yupc::GetDirectoryName(path);

    std::string::size_type const p(base.find_last_of('.'));
    auto woExt = base.substr(0, p);

    return directory + "/" + woExt + ".ll";
}

std::string yupc::GetBaseName(std::string const &path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

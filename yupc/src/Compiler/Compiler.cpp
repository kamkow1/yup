#include "Compiler/Compiler.h"
#include "Compiler/CompilationUnit.h"
#include "Compiler/Import.h"
#include "Compiler/Visitor.h"
#include "Compiler/FileSystem.h"
#include "Compiler/Configuration.h"
#include "utils.h"
#include "Logger.h"

#include "Parser/YupParser.h"
#include "Parser/ParserErrorListener.h"
#include "Lexer/YupLexer.h"
#include "Lexer/LexerErrorListener.h"

#include "llvm/IR/Verifier.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/MemoryBufferRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Linker/Linker.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <cstddef>
#include <system_error>
#include <vector>
#include <memory>
#include <cstring>

namespace fs = std::filesystem;

yupc::CompilerOptions yupc::GlobalCompilerOptions;

std::string yupc::CreateModuleName(std::string basePath)
{
    fs::path bd(yupc::GlobalBuildDirPath);
    fs::path f(yupc::GetBaseName(basePath));
    fs::path modulePath = bd / f;

    return yupc::CreateIrFileName(modulePath.string());
}

void yupc::ProcessSourceFile(std::string path) 
{
    std::string srcContent = yupc::ReadFileToString(path);

    antlr4::ANTLRInputStream input(srcContent);
    yupc::YupLexer lexer(&input);

    yupc::LexerErrorListener lexerErrorListener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lexerErrorListener);

    antlr4::CommonTokenStream tokens(&lexer);

    yupc::YupParser parser(&tokens);

    yupc::ParserErrorListener parserErrorListener;
    parser.removeErrorListeners();
    parser.addErrorListener(&parserErrorListener);

    yupc::YupParser::FileContext *ctx = parser.file();

    yupc::CompilationUnits.back()->ModuleName = yupc::CreateModuleName(path);
    yupc::CompilationUnits.back()->Context->setOpaquePointers(false);

    yupc::Visitor visitor;
    visitor.visit(ctx);

    verifyModule(*yupc::CompilationUnits.back()->Module, &llvm::errs());
    
    if (yupc::GlobalCompilerOptions.DumpIR)
    {
        yupc::DumpModuleToIrFile(yupc::CompilationUnits.back()->Module, 
                            yupc::CompilationUnits.back()->ModuleName);
    }
}

void yupc::BuildBitcode(fs::path bc_file) 
{
    yupc::GlobalLogger.LogCompilerInfo("finished linking LLVM bitcode! use the LLI command line tool to run it");

    std::error_code ec;
    llvm::raw_fd_ostream os(bc_file.string(), ec);
    llvm::WriteBitcodeToFile(*yupc::CompilationUnits.back()->Module, os);
}

void yupc::ProcessPath(std::string path) 
{
    std::string tempPath = path;
    std::vector<std::string> path_elems = yupc::SplitString(tempPath, '/');;
    for (size_t i = 0; i < tempPath.size(); i++)
    {
        for (auto &pv : yupc::GlobalPathVariables)
        {
            if (path_elems[i] == pv.first)
            {
                path = yupc::StringReplaceAll(path, pv.first, pv.second);
            }
        }
    }

    if (fs::path(path).extension().string() == ".yup")
    {
        yupc::CompilationUnit *compilationUnit = new yupc::CompilationUnit;
        yupc::InitializeCompilationUnit(compilationUnit, path);

        yupc::CompilationUnits.push_back(compilationUnit);

        yupc::ProcessSourceFile(path);
    }
    else if (fs::path(path).extension().string() == ".ll")
    {
        yupc::CompilationUnit *compilationUnit = new yupc::CompilationUnit;
        yupc::InitializeCompilationUnit(compilationUnit, path);

        llvm::LLVMContext *newContext = new llvm::LLVMContext;
        llvm::SMDiagnostic error;
        llvm::Module *importedModule = llvm::parseIRFile(path, error, *newContext).release();

        compilationUnit->Module = importedModule;
        compilationUnit->Context = newContext;

        yupc::CompilationUnits.push_back(compilationUnit);
    }
    else if (fs::path(path).extension().string() == ".bc")
    {
        yupc::CompilationUnit *compilationUnit = new yupc::CompilationUnit;
        yupc::InitializeCompilationUnit(compilationUnit, path);

        llvm::LLVMContext *newContext = new llvm::LLVMContext;
        llvm::MemoryBufferRef memory_buffer;        
        llvm::Module *importedModule = llvm::parseBitcodeFile(memory_buffer, *newContext)->release();

        compilationUnit->Module = importedModule;
        compilationUnit->Context = newContext;

        yupc::CompilationUnits.push_back(compilationUnit);
    }
}

void yupc::BuildProgram(yupc::CompilerOptions &options)
{
    for (std::string &path : options.SourcePaths)
    {
        if (fs::is_directory(path)) 
        {
            for (auto &entry : fs::directory_iterator(path)) 
            {
                if (!fs::is_directory(entry)) 
                {
                    yupc::ProcessPath(entry.path().string());
                }
            }
        } 
        else 
        {
            yupc::ProcessPath(path);
        }
    }

    std::string bc_file = yupc::InitializeBinDir();
    yupc::BuildBitcode(bc_file);
}

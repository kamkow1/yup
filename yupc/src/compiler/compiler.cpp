#include <compiler/compiler.h>
#include <compiler/compilation_unit.h>
#include <compiler/visitor.h>
#include <compiler/file_sys.h>
#include <compiler/config.h>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <memory>
#include <msg/errors.h>
#include <msg/info.h>

#include <parser/YupParser.h>
#include <parser/parser_error_listener.h>

#include <lexer/YupLexer.h>
#include <lexer/lexer_error_listener.h>

#include <string>
#include <filesystem>
#include <cstddef>
#include <system_error>
#include <vector>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Bitcode/BitcodeReader.h>

#define UNUSED(x) (void)(x)

namespace fs = std::filesystem;

yupc::CompilerOpts yupc::compiler_opts;

std::string yupc::create_module_name(std::string base_path)
{
    fs::path bd(yupc::build_dir);
    fs::path f(yupc::base_name(base_path));
    fs::path mod_path = bd / f;

    return yupc::get_ir_fname(mod_path.string());
}

void yupc::process_source_file(std::string path) 
{
    std::string src_content = yupc::file_to_str(path);

    antlr4::ANTLRInputStream input(src_content);
    yupc::YupLexer lexer(&input);

    yupc::LexerErrorListener lexer_error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lexer_error_listener);

    antlr4::CommonTokenStream tokens(&lexer);

    yupc::YupParser parser(&tokens);

    yupc::ParserErrorListener parser_error_listener;
    parser.removeErrorListeners();
    parser.addErrorListener(&parser_error_listener);

    yupc::YupParser::FileContext *ctx = parser.file();

    yupc::comp_units.back()->module_name = yupc::create_module_name(path);
    yupc::comp_units.back()->context->setOpaquePointers(false);

    yupc::Visitor visitor;
    visitor.visit(ctx);

    verifyModule(*yupc::comp_units.back()->module, &llvm::errs());
    dump_module(yupc::comp_units.back()->module, yupc::comp_units.back()->module_name);
}

void yupc::build_bitcode(fs::path bc_file) 
{
    for (size_t i = 1; i < yupc::comp_units.size(); i++)
    {
        llvm::Linker::linkModules(*yupc::comp_units.back()->module, 
            std::unique_ptr<llvm::Module>(yupc::comp_units[i]->module));
        yupc::comp_units.pop_back();
    }

    yupc::log_cmd_info("finished linking bitcode");
    std::error_code ec;
    llvm::raw_fd_ostream os(bc_file.string(), ec);
    llvm::WriteBitcodeToFile(*yupc::comp_units.back()->module, os);
}

void yupc::process_path(std::string path) 
{
    std::vector<std::string> path_elems;
    std::string tmp_path = path;
    boost::split(path_elems, tmp_path, boost::is_any_of("/"));
    for (size_t i = 0; i < tmp_path.size(); i++)
    {
        for (auto &pv : yupc::path_vars)
        {
            if (path_elems[i] == pv.first)
            {
                boost::replace_all(path, pv.first, pv.second);
            }
        }
    }

    if (fs::path(path).extension().string() == ".yup")
    {
        yupc::CompilationUnit *comp_unit = new yupc::CompilationUnit;
        yupc::init_comp_unit(*comp_unit, path);
        yupc::comp_units.push_back(comp_unit);

        yupc::process_source_file(path);
    }
    else if (fs::path(path).extension().string() == ".ll")
    {
        yupc::CompilationUnit *comp_unit = new yupc::CompilationUnit;
        yupc::init_comp_unit(*comp_unit, path);
        llvm::LLVMContext *new_ctx = new llvm::LLVMContext;
        llvm::SMDiagnostic error;
        llvm::Module *imported_mod = llvm::parseIRFile(path, error, *new_ctx).release();
        comp_unit->module = imported_mod;
        comp_unit->context = new_ctx;
        yupc::comp_units.push_back(comp_unit);
    }
    else if (fs::path(path).extension().string() == ".bc")
    {
        yupc::CompilationUnit *comp_unit = new yupc::CompilationUnit;
        yupc::init_comp_unit(*comp_unit, path);
        llvm::LLVMContext *new_ctx = new llvm::LLVMContext;
        llvm::MemoryBufferRef memory_buffer;        
        llvm::Module *imported_mod = llvm::parseBitcodeFile(memory_buffer, *new_ctx)->release();
        comp_unit->module = imported_mod;
        comp_unit->context = new_ctx;
        yupc::comp_units.push_back(comp_unit);
    }
}

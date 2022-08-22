#include <compiler/compiler.h>
#include <compiler/compilation_unit.h>
#include <compiler/visitor.h>
#include <compiler/file_sys.h>
#include <compiler/config.h>

#include <msg/errors.h>
#include <msg/info.h>

#include <parser/YupParser.h>
#include <parser/parser_error_listener.h>

#include <lexer/YupLexer.h>
#include <lexer/lexer_error_listener.h>

#include <string>
#include <filesystem>
#include <cstddef>
#include <vector>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>

#define UNUSED(x) (void)(x)

namespace fs = std::filesystem;

yupc::CompilerOpts yupc::compiler_opts;

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

    yupc::comp_units.back()->context->setOpaquePointers(false);

    yupc::YupParser::FileContext *ctx = parser.file();
    yupc::Visitor visitor;

    fs::path bd(yupc::build_dir);
    fs::path f(yupc::base_name(path));
    fs::path mod_path = bd / f;

    yupc::comp_units.back()->module_name = yupc::get_ir_fname(mod_path.string());

    //yupc::init_runtime_lib(*yupc::comp_units.back()->module);

    visitor.visit(ctx);

    // dump module to .ll
    //verifyModule(*yupc::comp_units.back()->module, &outs());
    dump_module(yupc::comp_units.back()->module, yupc::comp_units.back()->module_name);
}

void yupc::build_bitcode(fs::path bc_file, fs::path ll_dir) 
{

    std::string llvm_link = "llvm-link -o " + bc_file.string();
    for (fs::directory_entry entry : fs::directory_iterator(ll_dir)) 
    {
        if (!fs::is_directory(entry)) 
        {
            llvm_link += " " + entry.path().string();
        }
    }

    int r = std::system(llvm_link.c_str());
    UNUSED(r);
    yupc::log_cmd_info(llvm_link);


    if (!yupc::compiler_opts.give_perms) 
    {
        std::string chmod = (std::string("chmod +x ") + bc_file.string()).c_str();
        int rr = std::system(chmod.c_str());
        UNUSED(rr);
        yupc::log_cmd_info(chmod);
    }

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
}

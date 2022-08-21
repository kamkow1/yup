#include <compiler/compiler.h>
#include <compiler/compilation_unit.h>
#include <compiler/visitor.h>
#include <compiler/runtime_lib.h>

#include <filesystem>
#include <msg/errors.h>
#include <msg/info.h>

#include <parser/YupParser.h>
#include <parser/parser_error_listener.h>

#include <lexer/YupLexer.h>
#include <lexer/lexer_error_listener.h>

#include <string>
#include <util.h>
#include <dirent.h>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>

#define UNUSED(x) (void)(x)

std::string yupc::build_dir;
yupc::CompilerOpts yupc::compiler_opts;

void yupc::process_source_file(std::string path) 
{

    fs::path abs_src_path = fs::absolute(path);
    std::string src_content = yupc::file_to_str(abs_src_path);

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

    fs::path bd(build_dir);
    fs::path f(yupc::base_name(abs_src_path));
    fs::path mod_path = bd / f;

    yupc::comp_units.back()->module_name = yupc::get_ir_fname(mod_path.string());

    yupc::init_runtime_lib(*yupc::comp_units.back()->module);

    visitor.visit(ctx);

    // dump module to .ll
    //verifyModule(*yupc::comp_units.back()->module, &outs());
    dump_module(yupc::comp_units.back()->module, yupc::comp_units.back()->module_name);
}

std::string yupc::init_build_dir(std::string dir_base) 
{

    fs::path b(dir_base);
    fs::path bd("build");
    fs::path f_path = b / bd;

    if (!fs::is_directory(f_path.string()) || !fs::exists(f_path.string())) 
    {
        fs::create_directory(f_path.string());
    }

    return f_path.string();
}

std::string yupc::init_bin_dir() 
{
    fs::path bin(yupc::build_dir);
    fs::path dir("bin");

    fs::path bin_dir = bin / dir;
    fs::create_directory(bin_dir.string());

    fs::path p(yupc::compiler_opts.binary_name + ".bc");
    fs::path bc_file = bin_dir / p;

    return bc_file;
}

void yupc::dump_module(llvm::Module *module, std::string module_name) 
{
    std::error_code ec;
    llvm::raw_fd_ostream os(module_name, ec, llvm::sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    std::string info = "module " + yupc::comp_units.back()->module_name;
    yupc::log_cmd_info(info);
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
    yupc::CompilationUnit *comp_unit = new yupc::CompilationUnit;
    yupc::init_comp_unit(*comp_unit, path);
    yupc::comp_units.push_back(comp_unit);

    yupc::process_source_file(path);
}

#include <compiler/compiler.h>
#include <compiler/compilation_unit.h>
#include <compiler/visitor.h>

#include <msg/errors.h>
#include <msg/info.h>

#include <parser/YupParser.h>
#include <parser_error_listener.h>
#include <lexer/YupLexer.h>

#include <string>
#include <util.h>
#include <dirent.h>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>

using namespace yupc;
using namespace llvm;

namespace yu = util;
namespace cv = compiler::visitor;
namespace pse = parser_syntax_error;
namespace com_un = compiler::compilation_unit;

std::string compiler::build_dir;
compiler::CompilerOpts compiler::compiler_opts;

void compiler::process_source_file(std::string path) {

    std::string abs_src_path = fs::absolute(path);
    std::string src_content = yu::file_to_str(abs_src_path);

    antlr4::ANTLRInputStream input(src_content);
    lexer::YupLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::YupParser parser(&tokens);

    // add error listener
    pse::ParserErrorListener parserErrorListener;
    parser.removeErrorListeners();
    parser.addErrorListener(&parserErrorListener);

    parser::YupParser::FileContext* ctx = parser.file();
    cv::Visitor visitor;

    fs::path bd(build_dir);
    fs::path f(yu::base_name(abs_src_path));
    fs::path mod_path = bd / f;

    com_un::comp_units.top()->module_name = yu::get_ir_fname(mod_path.string());
    com_un::comp_units.top()->context->setOpaquePointers(false);

    visitor.visit(ctx);

    // dump module to .ll
    verifyModule(*com_un::comp_units.top()->module, &outs());
    dump_module(com_un::comp_units.top()->module.release(), 
        com_un::comp_units.top()->module_name);
        
    output_obj(com_un::comp_units.top()->module_name);
}

std::string compiler::init_build_dir(std::string dir_base) {

    fs::path b(dir_base);
    fs::path bd(".build");
    fs::path f_path = b / bd;
    bool succ = true;

    if (!fs::is_directory(f_path.string()) || !fs::exists(f_path.string())) {
        succ = fs::create_directory(f_path.string());
    }

    if (compiler_opts.verbose) {
        if (succ) {
            msg::info::log_cmd_info("successfully created .build directory");
        } else {
            msg::errors::log_cmd_err("failed to create .build directory");
        }
    }

    return f_path.string();
}

void compiler::dump_module(Module *module, std::string module_name) {
    
    std::error_code ec;
    raw_fd_ostream os(module_name, ec, sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    if (compiler::compiler_opts.verbose) {
        std::string info = "dumped module " + com_un::comp_units.top()->module_name;

        msg::info::log_cmd_info(info);
    }
}

void compiler::build_binary(fs::path bin_file, fs::path obj_dir) {

    std::string gcc = "gcc -o " + bin_file.string();
    for (const auto &entry : fs::directory_iterator(obj_dir)) {
        gcc += " " + entry.path().string();
    }

    std::system(gcc.c_str());

    if (compiler::compiler_opts.give_perms) {
        std::system((std::string("chmod +x ") + bin_file.string()).c_str());

        if (compiler::compiler_opts.verbose) {
            msg::info::log_cmd_info("gave permissions to the executable");
        }
    }

}

void compiler::output_obj(std::string s_path) {

    // -3 is .ll
    std::string binaryName = com_un::comp_units.top()->module_name.substr(0, 
        com_un::comp_units.top()->module_name.size() - 3);

    binaryName += ".o";
    fs::path d(yu::get_dir_name(s_path));
    fs::path b("obj");

    fs::path obj_dir = d / b;
    fs::create_directory(obj_dir.string());

    fs::path bn(yu::base_name(binaryName));
    std::string full_obj_name = obj_dir / bn;

    std::string obj_clang = "clang -c " + s_path
        + " -o " + full_obj_name
        + " -Wno-override-module";

    int ec = std::system(obj_clang.c_str());

    if (compiler::compiler_opts.verbose) {
        msg::info::log_cmd_info(obj_clang);
    }

    if (ec) {
        msg::errors::log_cmd_err(obj_clang);
    }

    fs::path bin(yu::get_dir_name(binaryName));
    fs::path dir("bin");

    fs::path bin_dir = bin / dir;
    fs::create_directory(bin_dir.string());

    fs::path p(yu::base_name(
        binaryName.substr(0, binaryName.size() - 2)));
    fs::path bin_file = bin_dir / p;

    compiler::build_binary(bin_file, obj_dir);
}

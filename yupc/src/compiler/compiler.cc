#include <compiler/compiler.h>
#include <compiler/compilation_unit.h>
#include <compiler/visitor.h>

#include <filesystem>
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

    auto abs_src_path = fs::absolute(path);
    auto src_content = yu::file_to_str(abs_src_path);

    antlr4::ANTLRInputStream input(src_content);
    lexer::YupLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::YupParser parser(&tokens);

    // add error listener
    pse::ParserErrorListener parserErrorListener;
    parser.removeErrorListeners();
    parser.addErrorListener(&parserErrorListener);

    auto *ctx = parser.file();
    cv::Visitor visitor;

    fs::path bd(build_dir);
    fs::path f(yu::base_name(abs_src_path));
    fs::path mod_path = bd / f;

    com_un::comp_units.back()->module_name = yu::get_ir_fname(mod_path.string());
    com_un::comp_units.back()->context->setOpaquePointers(false);

    visitor.visit(ctx);

    // dump module to .ll
    //verifyModule(*com_un::comp_units.back()->module, &outs());
    dump_module(com_un::comp_units.back()->module, com_un::comp_units.back()->module_name);
}

std::string compiler::init_build_dir(std::string dir_base) {

    fs::path b(dir_base);
    fs::path bd("build");
    fs::path f_path = b / bd;
    auto succ = true;

    if (!fs::is_directory(f_path.string()) || !fs::exists(f_path.string())) {
        succ = fs::create_directory(f_path.string());
    }

    if (compiler_opts.verbose) {
        if (succ) {
            msg::info::log_cmd_info("created build directory");
        } else {
            msg::errors::log_cmd_err("failed to create build directory");
        }
    }

    return f_path.string();
}

std::string compiler::init_bin_dir(std::string build_dir) {
    fs::path bin(compiler::build_dir);
    fs::path dir("bin");

    auto bin_dir = bin / dir;
    fs::create_directory(bin_dir.string());

    fs::path p(compiler::compiler_opts.binary_name + ".bc");
    auto bc_file = bin_dir / p;

    return bc_file;
}

void compiler::dump_module(Module *module, std::string module_name) {
    std::error_code ec;
    raw_fd_ostream os(module_name, ec, sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    if (compiler::compiler_opts.verbose) {
        std::string info = "dumped module " + com_un::comp_units.back()->module_name;

        msg::info::log_cmd_info(info);
    }
}

void compiler::build_bitcode(fs::path bc_file, fs::path ll_dir) {

    auto llvm_link = "llvm-link -o " + bc_file.string();
    for (const auto &entry : fs::directory_iterator(ll_dir)) {
        if (!fs::is_directory(entry)) {
            llvm_link += " " + entry.path().string();
        }
    }

    std::system(llvm_link.c_str());

    if (compiler::compiler_opts.give_perms) {
        std::system((std::string("chmod +x ") + bc_file.string()).c_str());

        if (compiler::compiler_opts.verbose) {
            msg::info::log_cmd_info("gave permissions to the executable");
        }
    }

}

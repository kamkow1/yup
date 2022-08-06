#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>

#include <lexer/YupLexer.h>
#include <parser/YupParser.h>
#include <msg/info.h>
#include <msg/errors.h>
#include <parser_error_listener.h>
#include <util.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <CLI/CLI.hpp>

#include <memory>
#include <string>
#include <filesystem>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace llvm;
using namespace CLI;
using namespace yupc;
using namespace yupc::msg::info;
using namespace yupc::msg::errors;

namespace fs = std::filesystem;
namespace cv = compiler::visitor;
namespace yu = yupc::util;
namespace pse = yupc::parser_syntax_error;
namespace msg = yupc::msg;
namespace com_un = compiler::compilation_unit;

struct CompilerOpts {
    bool emitIR;
    bool givePerms;
    bool verbose;

    std::string srcPath;
};

static CompilerOpts compiler_opts;

static void init_build_opts(App *build_cmd, CompilerOpts *compiler_opts) {
    build_cmd
        ->add_option("-s,--source", 
                compiler_opts->srcPath, 
                "path to a .yup file")
        ->required();

    build_cmd
        ->add_flag("--ir,--emit-ir", 
                compiler_opts->emitIR, 
                "enables emitting of the llvm intermediate representation");

    build_cmd
        ->add_flag("--np,--no-perm", 
                compiler_opts->givePerms, 
                "allows the compiler to give permissions to the binary file");

    build_cmd
        ->add_flag("-v,--verbose", 
                compiler_opts->verbose, 
                "enables verbose compiler output");
}

static std::string init_build_dir(std::string dir_base) {
    fs::path b(dir_base);
    fs::path bd(".build");
    fs::path f_path = b / bd;
    bool succ;

    if (!fs::is_directory(f_path.string()) || !fs::exists(f_path.string())) {
        succ = fs::create_directory(f_path.string());
    } else {
        succ = true;
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

static std::string build_dir;

static void dump_module(Module *module, std::string module_name) {
    std::error_code ec;
    raw_fd_ostream os(module_name, ec, sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    if (compiler_opts.verbose) {
        std::string info = "dumped module " + 
            com_un::comp_units.top().module_name;
        msg::info::log_cmd_info(info);
    }
}

static void build_binary(fs::path bin_file, fs::path obj_dir) {
    std::string gcc = "gcc -o " + bin_file.string();
    for (const auto &entry : fs::directory_iterator(obj_dir)) {
        gcc += " " + entry.path().string();
    }

    std::system(gcc.c_str());
}

static void output_obj(std::string s_path) {
    // -3 is .ll
    std::string binaryName = com_un::comp_units.top().module_name.substr(
        0, 
        com_un::comp_units.top().module_name.size() - 3);

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

    if (compiler_opts.verbose) {
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

    build_binary(bin_file, obj_dir);
}

static void process_source_file(std::string path) {
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

    com_un::comp_units.top().module_name 
        = yu::get_ir_fname(mod_path.string());

    std::cout << (com_un::comp_units.top().module_name) << "\n";

    com_un::comp_units.top()
        .context.setOpaquePointers(false);

    std::cout <<  "after\n";

    visitor.visit(ctx);

    // dump module to .ll
    verifyModule(*com_un::comp_units.top().module, &outs());
    dump_module(
        com_un::comp_units.top().module.release(), 
        com_un::comp_units.top().module_name);
        
    output_obj(com_un::comp_units.top().module_name);
}

static void init_comp_unit() {  
    com_un::CompilationUnit unit {
        "",
        LLVMContext{},
        IRBuilder<>(unit.context),
        std::make_unique<Module>(unit.module_name, unit.context),
        std::stack<std::map<std::string, AllocaInst*>>{},
        std::map<std::string, GlobalVariable*>{},
        std::stack<Value*>{},
        std::vector<std::string>{}
    };

    //com_un::comp_units.push(unit);
}

int main(int argc, char *argv[]) {
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", 
        "compiles a .yup source file into an executable binary");

    init_build_opts(build_cmd, &compiler_opts);

    build_cmd->callback([&]() {
        std::string fname = fs::absolute(compiler_opts.srcPath);
        
        init_comp_unit();
        build_dir = init_build_dir(yu::get_dir_name(fname));
        process_source_file(compiler_opts.srcPath); // entry point file

        /*
        for (size_t i = 0; i < com_un::comp_units.top()->module_imports.size(); i++) {
            std::string import_abs = fs::absolute(
                com_un::comp_units.top()->module_imports[i]);
            
            init_comp_unit(import_abs);
            process_source_file(import_abs);
        }
        */
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}

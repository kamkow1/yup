#include "compiler/visitor.h"
#include "lexer/YupLexer.h"
#include "parser/YupParser.h"
#include "msg/info.h"
#include "msg/errors.h"
#include "parser_error_listener.h"

#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "filesystem"
#include "string"

#include "CLI/CLI.hpp"
#include "string.h"
#include "filesystem"
#include "util.h"
#include "sys/ioctl.h"
#include "unistd.h"

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

struct CompilerOpts
{
    bool emitIR;
    bool givePerms;
    bool verbose;
    bool outputObj;

    std::string srcPath;
};

static CompilerOpts compiler_opts;

static void init_build_opts(App *build_cmd, CompilerOpts *compiler_opts)
{
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

    build_cmd
        ->add_flag("-o, --object", 
                compiler_opts->outputObj, 
                "outputs .o object file instead of an executable ~ doesn't require main()");
}

static std::string init_build_dir(std::string dir_base)
{
    fs::path b(dir_base);
    fs::path bd(".build");
    fs::path f_path = b / bd;
    bool succ =fs::create_directory(f_path.string());

    if (compiler_opts.verbose)
    {
        if (succ)
        {
            msg::info::log_cmd_info("successfully created .build directory");
        }
        else
        {
            msg::info::log_cmd_info("failed to create .build directory");
        }
    }

    return f_path.string();
}

static void dump_module(Module *module, std::string module_name)
{
    std::error_code ec;
    raw_fd_ostream os(module_name, ec, sys::fs::OF_None);
    module->print(os, nullptr);
    os.flush();

    if (compiler_opts.verbose)
    {
        std::string info = "dumped module " + cv::module_name;
        msg::info::log_cmd_info(info);
    }
}

static void output_o_or_bin(std::string s_path)
{
    // -3 is .ll
    std::string binaryName = cv::module_name.substr(0, 
        cv::module_name.size() - 3);

    if (compiler_opts.outputObj)
    {
        binaryName += ".o";
        std::string obj_clang = "clang -c " + s_path;
        int ec = std::system(obj_clang.c_str());

        if (compiler_opts.verbose)
        {
            msg::info::log_cmd_info(obj_clang);
        }

        if (ec)
        {
            msg::errors::log_cmd_err(obj_clang);
        }
    }
    else
    {
        fs::path d(yu::get_dir_name(s_path));
        fs::path b("bin");

        fs::path bin_dir = d / b;
        fs::create_directory(bin_dir.string());

        fs::path bn(yu::base_name(binaryName));
        std::string full_bin_name = bin_dir / bn;

        std::string bin_clang = "clang " + s_path 
            + " -o " + full_bin_name
            + " -Wno-override-module";
        int ec = std::system(bin_clang.c_str());

        if (compiler_opts.verbose)
        {
            msg::info::log_cmd_info(bin_clang);
        }

        if (ec)
        {
            msg::errors::log_cmd_err(bin_clang);
        }
    }
}

int main(int argc, char *argv[]) 
{
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");

    init_build_opts(build_cmd, &compiler_opts);

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(compiler_opts.srcPath);
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

        std::string build_dir = init_build_dir(yu::get_dir_name(abs_src_path));
        fs::path bd(build_dir);
        fs::path f(yu::base_name(abs_src_path));
        fs::path mod_path = bd / f;

        cv::module_name = yu::get_ir_fname(mod_path.string());

        cv::context.setOpaquePointers(false);
        visitor.visit(ctx);

        // dump module to .ll
        verifyModule(*cv::module, &outs());
        dump_module(cv::module.release(), cv::module_name);
        
        output_o_or_bin(cv::module_name);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
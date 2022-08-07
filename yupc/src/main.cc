#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/compiler.h>

#include <lexer/YupLexer.h>
#include <llvm/IR/IRBuilder.h>
#include <parser/YupParser.h>
#include <msg/info.h>
#include <msg/errors.h>
#include <parser_error_listener.h>
#include <stack>
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
#include <utility>

using namespace llvm;
using namespace CLI;
using namespace yupc;
using namespace yupc::msg::info;
using namespace yupc::msg::errors;

namespace fs = std::filesystem;
namespace yu = yupc::util;
namespace com_un = compiler::compilation_unit;

void init_build_opts(App *build_cmd, compiler::CompilerOpts *compiler_opts) {
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

int main(int argc, char *argv[]) {
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", 
        "compiles a .yup source file into an executable binary");

    init_build_opts(build_cmd, &compiler::compiler_opts);

    build_cmd->callback([&]() {
        std::string fname = fs::absolute(compiler::compiler_opts.srcPath);

        com_un::CompilationUnit comp_unit {
            "", new LLVMContext, IRBuilder<> (*comp_unit.context),
            std::make_unique<Module>(comp_unit.module_name, *comp_unit.context),
            std::stack<std::map<std::string, AllocaInst*>>{},
            std::map<std::string, GlobalVariable*>{},
            std::stack<Value*>{}, std::vector<std::string>{}
        };

        com_un::comp_units.push(&comp_unit);

        compiler::build_dir = compiler::init_build_dir(yu::get_dir_name(fname));

        compiler::process_source_file(compiler::compiler_opts.srcPath);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}

#include <compiler/visitor.h>
#include <compiler/compilation_unit.h>
#include <compiler/compiler.h>
#include <compiler/import.h>
#include <compiler/type.h>

#include <lexer/YupLexer.h>
#include <parser/YupParser.h>
#include <parser_error_listener.h>

#include <msg/info.h>
#include <msg/errors.h>

#include <util.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <CLI/CLI.hpp>

#include <string>
#include <filesystem>
#include <stack>

using namespace llvm;
using namespace CLI;
using namespace yupc;
using namespace yupc::msg::info;
using namespace yupc::msg::errors;

namespace fs = std::filesystem;
namespace com_un = compiler::compilation_unit;

void init_build_opts(App *build_cmd, compiler::CompilerOpts *compiler_opts) {
    build_cmd->add_option("-s,--sources", compiler_opts->src_path, ".yup source files");

    build_cmd->add_option("-b,--binary-name", compiler_opts->binary_name, "sets the output binary's name");

    build_cmd->add_flag("-n,--no-perm", compiler_opts->give_perms, "allows the compiler to give permissions to the binary file");

    build_cmd->add_flag("-v,--verbose", compiler_opts->verbose, "enables verbose compiler output");
}

void process_path(std::string path) {
    auto *comp_unit = new com_un::CompilationUnit;
    com_un::init_comp_unit(*comp_unit);
    com_un::comp_units.push_back(comp_unit);

    compiler::process_source_file(path);
}

int main(int argc, char *argv[]) {
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", 
        "compiles a .yup source file into an executable binary");

    init_build_opts(build_cmd, &compiler::compiler_opts);

    build_cmd->callback([&]() {
        auto cwd = fs::current_path();
        compiler::build_dir = compiler::init_build_dir(cwd.string());

        for (auto &path : compiler::compiler_opts.src_path) {

            if (fs::is_directory(path)) {
                for (auto &entry : fs::directory_iterator(path)) {
                    if (!fs::is_directory(entry)) {
                        process_path(entry.path().string());
                    }
                }

            } else {

                process_path(path);
            }
        }

        auto bc_file = compiler::init_bin_dir(compiler::build_dir);
        compiler::build_bitcode(bc_file, compiler::build_dir);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}

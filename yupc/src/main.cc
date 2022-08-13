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

#include <memory>
#include <string>
#include <filesystem>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>
#include <stack>
#include <cstddef>

using namespace llvm;
using namespace CLI;
using namespace yupc;
using namespace yupc::msg::info;
using namespace yupc::msg::errors;

namespace fs = std::filesystem;
namespace yu = yupc::util;
namespace com_un = compiler::compilation_unit;
namespace ci = compiler::import;
namespace ct = compiler::type;

void init_build_opts(App *build_cmd, compiler::CompilerOpts *compiler_opts) {
    build_cmd->add_option("-s,--sources", compiler_opts->src_path, ".yup source files");

    build_cmd->add_option("-b,--binary-name", compiler_opts->binary_name, "sets the output binary's name");

    build_cmd->add_flag("-n,--no-perm", compiler_opts->give_perms, "allows the compiler to give permissions to the binary file");

    build_cmd->add_flag("-v,--verbose", compiler_opts->verbose, "enables verbose compiler output");
}

int main(int argc, char *argv[]) {
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", 
        "compiles a .yup source file into an executable binary");

    init_build_opts(build_cmd, &compiler::compiler_opts);

    build_cmd->callback([&]() {
        for (auto &path : compiler::compiler_opts.src_path) {
            auto fname = fs::absolute(path);

            auto *comp_unit = new com_un::CompilationUnit; 
            comp_unit->module_id = "";
            comp_unit->module_name = "";
            comp_unit->context = new LLVMContext;
            comp_unit->ir_builder = new IRBuilder<>(*comp_unit->context);
            comp_unit->module = new Module(comp_unit->module_name, *comp_unit->context);
            comp_unit->symbol_table = std::vector<std::map<std::string, AllocaInst*>>{};
            comp_unit->global_variables = std::map<std::string, GlobalVariable*>{};
            comp_unit->functions = std::map<std::string, Function*>{};
            comp_unit->value_stack = std::stack<Value*>{};
            comp_unit->imported_syms = std::vector<ci::ImportDecl>{};
            comp_unit->alias_types = std::vector<ct::AliasType*>{};

            com_un::comp_units.push_back(comp_unit);

            compiler::build_dir = compiler::init_build_dir(yu::get_dir_name(fname));

            compiler::process_source_file(path);
        }


        fs::path bin(compiler::build_dir);
        fs::path dir("bin");

        auto bin_dir = bin / dir;
        fs::create_directory(bin_dir.string());

        fs::path p(compiler::compiler_opts.binary_name + ".bc");
        auto bin_file = bin_dir / p;

        compiler::build_bitcode(bin_file, compiler::build_dir);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}

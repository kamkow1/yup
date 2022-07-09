#include <CLI/CLI.hpp>
#include <YupLexer.h>
#include <YupParser.h>
#include <string>
#include <filesystem>
#include <visitor.h>
#include <util.h>
#include <messaging/information.h>
#include <parser_error_listener.h>
#include <compiler_options.h>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) 
{
    CLI::App cli{"a compiler for the yup programming language"};

    auto build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");

    std::string src_path;
    build_cmd
        ->add_option("-s,--source", src_path, "path to a .yup file")
        ->required();

    build_cmd
        ->add_option("-t,--target", targetName, "specifies assembly target for clang")
        ->required();

    build_cmd
        ->add_flag("--ir,--emit-ir", emitIR, "enables emitting of the llvm intermediate representation");

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(src_path);
        std::string src_content = fileToString(abs_src_path);

        std::string targetInfo = "selected target " + targetName;
        logCommandInformation(targetInfo);

        std::string commandInfo = "compiling source file " + abs_src_path;
        logCommandInformation(commandInfo);

        antlr4::ANTLRInputStream input(src_content);
        YupLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        YupParser parser(&tokens);

        // add error listener
        ParserErrorListener parserErrorListener;
        parser.removeErrorListeners();
        parser.addErrorListener(&parserErrorListener);

        YupParser::FileContext* ctx = parser.file();

        Visitor visitor;

        moduleName = getIRFileName(abs_src_path);
        visitor.visit(ctx);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
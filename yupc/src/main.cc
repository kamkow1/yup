#include <CLI/CLI.hpp>
#include "lexer/YupLexer.h"
#include "parser/YupParser.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <visitor.h>
#include <thread>

namespace fs = std::filesystem;

std::string fileToString(const std::string& path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open()) 
    {
        std::cerr << "Could not open the file - '"
             << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string((
        std::istreambuf_iterator<char>(input_file)), 
        std::istreambuf_iterator<char>());
}

int main(int argc, char *argv[]) 
{
    CLI::App cli{"a compiler for the yup programming language"};

    auto build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");

    std::string src_path;
    build_cmd
        ->add_option("-s,--source", src_path, "path to a .yup file")
        ->required();

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(src_path);
        std::string src_content = fileToString(abs_src_path);

        antlr4::ANTLRInputStream input(src_content);
        YupLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        YupParser parser(&tokens);

        YupParser::FileContext* ctx = parser.file();

        Visitor visitor;

        moduleName = abs_src_path;
        std::cout << "current thread id: " << std::this_thread::get_id() << "\n";
        visitor.visit(ctx);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
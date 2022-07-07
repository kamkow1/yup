#include "CLI/CLI.hpp"
#include "lexer/YupLexer.h"
#include "parser/YupParser.h"
#include "ast_visitor/visitor.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

namespace fs = std::filesystem;

std::string file_to_str(const std::string& path) 
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
    CLI::App yupc{"a compiler for the yup programming language"};

    auto build_cmd = yupc.add_subcommand("build", "compiles a .yup source file into an executable binary");

    std::string src_path;
    build_cmd
        ->add_option("-s,--source", src_path, "path to a .yup file")
        ->required();

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(src_path);
        std::string src_content = file_to_str(abs_src_path);

        antlr4::ANTLRInputStream input(src_content);
        YupLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        YupParser parser(&tokens);

        YupParser::FileContext* ctx = parser.file();

        Visitor visitor;
        visitor.visit(ctx);
    });

    CLI11_PARSE(yupc, argc, argv);

    return 0;
}
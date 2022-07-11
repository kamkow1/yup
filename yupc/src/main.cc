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
        ->add_option("-a,--arch", archName, "specifies targeted cpu's assembly for clang");

    build_cmd
        ->add_option("-t,--target", targetName, "specifies target for clang");

    build_cmd
        ->add_option("-m,--mmcu", mmcu, "specifies target microcontroller (when --target == avr)");

    build_cmd
        ->add_flag("--ir,--emit-ir", emitIR, "enables emitting of the llvm intermediate representation");

    build_cmd
        ->add_flag("--np,--no-perm", givePermissions, "allows the compiler to give permissions to the binary file");

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(src_path);
        std::string src_content = fileToString(abs_src_path);

        std::string targetInfo = "selected architecture " + archName;
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

        // -3 is .ll
        std::string binaryName = moduleName.substr(0, moduleName.size() - 3);

        std::string clangCommand =
                "clang --output " + binaryName + " " + moduleName
                + " -march=" + archName
                + (targetName.length() > 0 ? " --target=" + targetName : "")
                + (mmcu.length() > 0 ? " -mmcu="+ mmcu : "")
                // clang flags
                + " -Wno-override-module"
                + " -Wno-unused-command-line-argument";

        logCommandInformation(clangCommand);
        int result = std::system(clangCommand.c_str());

        std::string resultInfo = "compiled to " + binaryName + " with status code " + std::to_string(result);
        logCommandInformation(resultInfo);

        // remove the .ll file
        if (!emitIR)
        {
            std::string cleanupCommand = "rm -f " + moduleName;
            std::system(cleanupCommand.c_str());
            logCommandInformation(cleanupCommand);
        }

        if (!givePermissions)
        {
            std::string permCommand = "chmod +x " + binaryName;
            std::system(permCommand.c_str());
            logCommandInformation(permCommand);
        }

        llvm::verifyModule(*module, &llvm::outs());
        std::error_code ec;
        llvm::raw_fd_ostream os(moduleName, ec, llvm::sys::fs::OF_None);
        module->print(os, nullptr);
        os.flush();

        std::string info = "dumped module " + moduleName;
        logCommandInformation(info);
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
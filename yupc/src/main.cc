#include "CLI/CLI.hpp"
#include "YupLexer.h"
#include "YupParser.h"
#include "string.h"
#include "filesystem"
#include "visitor.h"
#include "util.h"
#include "messaging/information.h"
#include "parser_error_listener.h"
#include "sys/ioctl.h"
#include "unistd.h"

using namespace llvm;
using namespace CLI;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) 
{
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");

    std::string src_path;
    build_cmd
        ->add_option("-s,--source", src_path, "path to a .yup file")
        ->required();

    std::string archName;
    build_cmd
        ->add_option("-a,--arch", archName, "specifies targeted cpu's assembly for clang");

    std::string targetName;
    build_cmd
        ->add_option("-t,--target", targetName, "specifies target for clang");

    bool emitIR;
    build_cmd
        ->add_flag("--ir,--emit-ir", emitIR, "enables emitting of the llvm intermediate representation");

    bool givePermissions;
    build_cmd
        ->add_flag("--np,--no-perm", givePermissions, "allows the compiler to give permissions to the binary file");

    bool verbose;
    build_cmd
        ->add_flag("-v, --verbose", verbose, "enables verbose compiler output");

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(src_path);
        std::string src_content = fileToString(abs_src_path);

        antlr4::ANTLRInputStream input(src_content);
        YupLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        YupParser parser(&tokens);

        // add error listener
        ParserErrorListener parserErrorListener;
        parser.removeErrorListeners();
        parser.addErrorListener(&parserErrorListener);

        context.setOpaquePointers(false);

        YupParser::FileContext* ctx = parser.file();

        Visitor visitor;

        moduleName = getIRFileName(abs_src_path);
        visitor.visit(ctx);

        // dump module to .ll
        verifyModule(*module, &outs());
        std::error_code ec;
        raw_fd_ostream os(moduleName, ec, sys::fs::OF_None);
        module->print(os, nullptr);
        os.flush();

        if (verbose)
        {
            std::string info = "dumped module " + moduleName;
            logCommandInformation(info);
        }

        // -3 is .ll
        std::string binaryName = moduleName.substr(0, moduleName.size() - 3);

        std::string clangCommand =
                "clang --output " + binaryName + " " + moduleName
                + " -march=" + archName
                + (targetName.length() > 0 ? " --target=" + targetName : "")
                // clang flags
                + " -Wno-deprecated-declarations"
                + " -Wno-override-module"
                + " -Wno-unused-command-line-argument";

        if (verbose)
        {
            logCommandInformation(clangCommand);
        }

        int result = std::system(clangCommand.c_str());

        if (verbose)
        {
            std::string resultInfo = "compiled to " + binaryName + " with status code " + std::to_string(result);
            logCommandInformation(resultInfo);
        }


        // remove the .ll file
        if (!emitIR)
        {
            std::string cleanupCommand = "rm -f " + moduleName;
            int cleanResult = std::system(cleanupCommand.c_str());
            if (cleanResult != 0)
            {
                logCommandError("failed to execute cmd: " + cleanupCommand);
                exit(1);
            }
            
            if (verbose)
            {
                logCommandInformation(cleanupCommand);
            }
        }

        if (!givePermissions)
        {
            std::string permCommand = "chmod +x " + binaryName;
            int premResult = std::system(permCommand.c_str());
            if (premResult != 0)
            {
                logCommandError("failed to give permissions to the binary: " + permCommand);
                exit(1);
            }
            
            if (verbose)
            {
                logCommandInformation(permCommand);
            }
        }
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
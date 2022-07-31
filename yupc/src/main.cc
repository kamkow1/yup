#include "CLI/CLI.hpp"
#include "YupLexer.h"
#include "YupParser.h"
#include "string.h"
#include "filesystem"
#include "compiler/visitor.h"
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
        ->add_option("-s,--source", 
                src_path, 
                "path to a .yup file")
        ->required();

    bool emitIR;
    build_cmd
        ->add_flag("--ir,--emit-ir", 
                emitIR, 
                "enables emitting of the llvm intermediate representation");

    bool givePermissions;
    build_cmd
        ->add_flag("--np,--no-perm", 
                givePermissions, 
                "allows the compiler to give permissions to the binary file");

    bool verbose;
    build_cmd
        ->add_flag("-v,--verbose", 
                verbose, 
                "enables verbose compiler output");

    bool outputObj;
    build_cmd
        ->add_flag("-o, --object", 
                outputObj, 
                "outputs .o object file instead of an executable ~ doesn't require main()");

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

        std::string llcCommand = "llc " + moduleName;

        if (verbose)
        {
            logCommandInformation(llcCommand);
        }

        int llcResult = std::system(llcCommand.c_str());
        if (llcResult != 0)
        {
            logCommandError("failed " + llcCommand);
            exit(1);
        }

        if (verbose)
        {
            std::string resultInfo = "compiled to " + binaryName 
                + ".o" + " with status code " + std::to_string(llcResult);
            logCommandInformation(resultInfo);
        }

        if (outputObj) // output .o
        {
            std::string gccCommand = "gcc -c " + binaryName + ".s" + " -o " + binaryName + ".o";

            int gccResult = std::system(gccCommand.c_str());
            if (gccResult != 0)
            {
                logCommandError("failed " + gccCommand);
                exit(1);
            }

            if (verbose)
            {
                logCommandInformation(gccCommand);
            }
        }
        else // output executable
        {
            std::string gccCommand = "gcc " + binaryName 
            + ".s" + " -o " + binaryName;

            int gccResult = std::system(gccCommand.c_str());
            if (gccResult != 0)
            {
                logCommandError("failed " + gccCommand);
                exit(1);
            }

            if (verbose)
            {
                logCommandInformation(gccCommand);
            }
        }

        std::string cleanupCommand = "rm -f " + binaryName + ".s";
        if (verbose)
        {
            logCommandInformation(cleanupCommand);
        }

        int cleanupResult = std::system(cleanupCommand.c_str());
        if (cleanupResult != 0)
        {
            logCommandError("failed " + cleanupCommand);
            exit(1);
        }

        // remove the .ll file
        if (!emitIR)
        {
            std::string cleanupCommand = "rm -f " + moduleName;
            int cleanResult = std::system(cleanupCommand.c_str());
            if (cleanResult != 0)
            {
                logCommandError("failed " + cleanupCommand);
                exit(1);
            }
            
            if (verbose)
            {
                logCommandInformation(cleanupCommand);
            }
        }

        if (!givePermissions && !outputObj)
        {
            std::string permCommand = "chmod +x " + binaryName;
            int premResult = std::system(permCommand.c_str());
            if (premResult != 0)
            {
                logCommandError("failed " + permCommand);
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
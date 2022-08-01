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

struct CompilerOpts
{
    bool emitIR;
    bool givePerms;
    bool verbose;
    bool outputObj;

    std::string srcPath;
};

int main(int argc, char *argv[]) 
{
    App cli{"a compiler for the yup programming language"};

    App *build_cmd = cli.add_subcommand("build", "compiles a .yup source file into an executable binary");

    CompilerOpts compilerOpts;

    build_cmd
        ->add_option("-s,--source", 
                compilerOpts.srcPath, 
                "path to a .yup file")
        ->required();

    build_cmd
        ->add_flag("--ir,--emit-ir", 
                compilerOpts.emitIR, 
                "enables emitting of the llvm intermediate representation");

    build_cmd
        ->add_flag("--np,--no-perm", 
                compilerOpts.givePerms, 
                "allows the compiler to give permissions to the binary file");

    build_cmd
        ->add_flag("-v,--verbose", 
                compilerOpts.verbose, 
                "enables verbose compiler output");

    build_cmd
        ->add_flag("-o, --object", 
                compilerOpts.outputObj, 
                "outputs .o object file instead of an executable ~ doesn't require main()");

    build_cmd->callback([&]() 
    {
        std::string abs_src_path = fs::absolute(compilerOpts.srcPath);
        std::string src_content = file_to_str(abs_src_path);

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

        module_name = get_ir_fname(abs_src_path);
        visitor.visit(ctx);

        // dump module to .ll
        verifyModule(*module, &outs());
        std::error_code ec;
        raw_fd_ostream os(module_name, ec, sys::fs::OF_None);
        module->print(os, nullptr);
        os.flush();

        if (compilerOpts.verbose)
        {
            std::string info = "dumped module " + module_name;
            log_cmd_info(info);
        }

        // -3 is .ll
        std::string binaryName = module_name.substr(0, module_name.size() - 3);

        std::string llcCommand = "llc " + module_name;

        if (compilerOpts.verbose)
        {
            log_cmd_info(llcCommand);
        }

        int llcResult = std::system(llcCommand.c_str());
        if (llcResult != 0)
        {
            log_cmd_err("failed " + llcCommand);
            exit(1);
        }

        if (compilerOpts.verbose)
        {
            std::string resultInfo = "compiled to " + binaryName 
                + ".o" + " with status code " + std::to_string(llcResult);
            log_cmd_info(resultInfo);
        }

        if (compilerOpts.verbose) // output .o
        {
            std::string gccCommand = "gcc -c " + binaryName + ".s" + " -o " + binaryName + ".o";

            int gccResult = std::system(gccCommand.c_str());
            if (gccResult != 0)
            {
                log_cmd_err("failed " + gccCommand);
                exit(1);
            }

            if (compilerOpts.verbose)
            {
                log_cmd_info(gccCommand);
            }
        }
        else // output executable
        {
            std::string gccCommand = "gcc " + binaryName 
            + ".s" + " -o " + binaryName;

            int gccResult = std::system(gccCommand.c_str());
            if (gccResult != 0)
            {
                log_cmd_err("failed " + gccCommand);
                exit(1);
            }

            if (compilerOpts.verbose)
            {
                log_cmd_info(gccCommand);
            }
        }

        std::string cleanupCommand = "rm -f " + binaryName + ".s";
        if (compilerOpts.verbose)
        {
            log_cmd_info(cleanupCommand);
        }

        int cleanupResult = std::system(cleanupCommand.c_str());
        if (cleanupResult != 0)
        {
            log_cmd_err("failed " + cleanupCommand);
            exit(1);
        }

        // remove the .ll file
        if (!compilerOpts.emitIR)
        {
            std::string cleanupCommand = "rm -f " + module_name;
            int cleanResult = std::system(cleanupCommand.c_str());
            if (cleanResult != 0)
            {
                log_cmd_err("failed " + cleanupCommand);
                exit(1);
            }
            
            if (compilerOpts.verbose)
            {
                log_cmd_info(cleanupCommand);
            }
        }

        if (!compilerOpts.givePerms && !compilerOpts.outputObj)
        {
            std::string permCommand = "chmod +x " + binaryName;
            int premResult = std::system(permCommand.c_str());
            if (premResult != 0)
            {
                log_cmd_err("failed " + permCommand);
                exit(1);
            }
            
            if (compilerOpts.verbose)
            {
                log_cmd_info(permCommand);
            }
        }
    });

    CLI11_PARSE(cli, argc, argv);

    return 0;
}
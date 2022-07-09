#include <visitor.h>
#include <messaging/errors.h>
#include <messaging/information.h>
#include <compiler_options.h>

std::any Visitor::visitCompiler_flag(YupParser::Compiler_flagContext *ctx)
{
    // match compiler flag name
    std::string flagName = ctx->IDENTIFIER()->getText();

    if (flagName == "dump")
    {
        llvm::verifyModule(*module, &llvm::outs());
        std::error_code ec;
        llvm::raw_fd_ostream os(moduleName, ec, llvm::sys::fs::OF_None);
        module->print(os, nullptr);
        os.flush();

        std::string info = "dumped module " + moduleName;
        logCommandInformation(info);

        return nullptr;
    }

    if (flagName == "auto_compile")
    {
        // -3 is .ll
        std::string binaryName = moduleName.substr(0, moduleName.size() - 3);

        std::string clangCommand =
                "clang --output "
                + binaryName
                + " "
                + moduleName
                + " -march="
                + targetName
                + " -Wno-override-module";

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

        std::string permCommand = "chmod +x " + binaryName;
        std::system(permCommand.c_str());
        logCommandInformation(permCommand);

        return nullptr;
    }

    std::string errorMessage = "unknown compiler flag \"" + flagName + "\"";
    logCompilerError(errorMessage);
    exit(1);
}
#include <visitor.h>
#include <messaging/errors.h>

std::any Visitor::visitCompiler_flag(YupParser::Compiler_flagContext *ctx)
{
    // match compiler flag name
    std::string flagName = ctx->IDENTIFIER()->getText();

    if (flagName == "end")
    {
        llvm::verifyModule(*module, &llvm::outs());
        std::error_code ec;
        llvm::raw_fd_ostream os(moduleName, ec, llvm::sys::fs::OF_None);
        module->print(os, nullptr);
        os.flush();
        return nullptr;
    }

    std::string errorMessage = "unknown compiler flag \"" + flagName + "\"";
    logCompilerError(errorMessage);
    exit(1);
}
#include <visitor.h>

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

    fprintf(stderr, "ERROR: unknown compiler flag %s\n", flagName.c_str());
    exit(1);
}
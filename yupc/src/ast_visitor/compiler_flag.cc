#include <visitor.h>

std::any Visitor::visitCompiler_flag(YupParser::Compiler_flagContext *ctx)
{
    // match compiler flag name
    std::string flagName = ctx->IDENTIFIER()->getText();
    std::cout << flagName << "\n";

    if (flagName == "end")
    {
        llvm::verifyModule(*module, &llvm::outs());
        llvm::raw_string_ostream os(moduleName);
        module->print(llvm::outs(), nullptr);
        os.flush();
    }

    return nullptr;
}
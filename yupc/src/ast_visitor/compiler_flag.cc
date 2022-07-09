#include <visitor.h>
#include <thread>

std::any Visitor::visitCompiler_flag(YupParser::Compiler_flagContext *ctx)
{
    // match compiler flag name
    std::string flagName = ctx->IDENTIFIER()->getText();
    std::cout << flagName << "\n";

    if (flagName == "end")
    {
        std::cout << "current thread id: " << std::this_thread::get_id() << "\n";
        if (!module->isMaterialized())
        {
            std::cout << "module is empty\n";
        }
    }

    return nullptr;
}
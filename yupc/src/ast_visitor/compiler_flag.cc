#include <visitor.h>
#include <messaging/errors.h>
#include <messaging/information.h>
#include <compiler_options.h>

std::any Visitor::visitCompiler_flag(YupParser::Compiler_flagContext *ctx)
{
    // match compiler flag name
    std::string flagName = ctx->IDENTIFIER()->getText();

    std::string errorMessage = "unknown compiler flag \"" + flagName + "\"";
    logCompilerError(errorMessage);
    exit(1);
}
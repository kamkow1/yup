#include "visitor.h"
#include "util.h"
#include "compiler/type.h"

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    for (int i = 0; i < ctx->ASTERISK().size(); i++)
    {
        name += "*";
    }

    if (ctx->AMPERSAND() != nullptr)
    {
        name += "&";
    }

    TypeAnnotation ta = TypeAnnotation{name};
    return ta;
}
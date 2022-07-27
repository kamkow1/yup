#include "visitor.h"
#include "util.h"
#include "compiler/type.h"

static std::string createTypeName(std::string base)
{

}

std::any Visitor::visitType_annot(YupParser::Type_annotContext *ctx)
{
    std::string name = ctx->type_name()->getText();

    TypeAnnotation ta = TypeAnnotation{name};
    return ta;
}
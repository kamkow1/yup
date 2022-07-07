#pragma once
#include "../parser/YupParserBaseVisitor.h"

class Visitor : public YupParserBaseVisitor 
{
public:
    std::any visitFunc_def(YupParser::Func_defContext *ctx) override;

    std::any visitFile(YupParser::FileContext *ctx) override;
};
#pragma once

#include <BaseErrorListener.h>
#include <antlr4-runtime.h>

namespace yupc 
{
    class ParserErrorListener : public antlr4::BaseErrorListener 
    {
    public:
        virtual void syntaxError(antlr4::Recognizer *recognizer,
                                antlr4::Token *offendingSymbol,
                                size_t line, size_t charPositionInLine,
                                const std::string &msg,
                                std::exception_ptr e) override;
    };
}

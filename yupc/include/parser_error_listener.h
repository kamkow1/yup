#pragma once

#include "antlr4-runtime.h"
#include "parser/YupParser.h"

namespace yupc::parser_syntax_error {
    class ParserErrorListener : public antlr4::BaseErrorListener {
    public:
        virtual void syntaxError(antlr4::Recognizer *recognizer,
                                antlr4::Token *offendingSymbol,
                                size_t line, size_t charPositionInLine,
                                const std::string &msg,
                                std::exception_ptr e) override;
    };
}

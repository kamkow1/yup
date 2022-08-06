#include "msg/errors.h"
#include "parser_error_listener.h"
#include "string.h"

using namespace yupc;
using namespace yupc::msg::errors;


namespace pse = parser_syntax_error;

void pse::ParserErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                     antlr4::Token *offendingSymbol,
                                     size_t line,
                                     size_t charPositionInLine,
                                     const std::string &msg,
                                     std::exception_ptr e) {
    std::string errorMessage =
            "unexpected token \""
            + offendingSymbol->getText() + "\" "
            + "in line no. "
            + std::to_string(offendingSymbol->getLine())
            + " at position "
            + std::to_string(charPositionInLine);

    log_parsing_err(errorMessage);
    exit(1);
}

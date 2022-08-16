#include <compiler/compilation_unit.h>

#include <msg/errors.h>
#include <parser/parser_error_listener.h>
#include <string>

using namespace yupc;
using namespace yupc::msg::errors;


namespace pse = parser_syntax_error;
namespace com_un = compiler::compilation_unit;

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

    log_parsing_err(errorMessage, com_un::comp_units.back()->source_file);
    exit(1);
}

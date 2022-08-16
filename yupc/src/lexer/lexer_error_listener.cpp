#include <compiler/compilation_unit.h>
#include <lexer/lexer_error_listener.h>
#include <msg/errors.h>
#include <string>

using namespace yupc;

namespace lse = lexer::lexer_syntax_error;
namespace com_un = compiler::compilation_unit;

void lse::LexerErrorListener::syntaxError(antlr4::Recognizer *recognizer, 
                                        antlr4::Token *offendingSymbol, 
                                        size_t line, 
                                        size_t charPositionInLine, 
                                        const std::string &msg, 
                                        std::exception_ptr e) {

    auto error_message = "unable to recognize token \""
                        + offendingSymbol->getText() + "\" "
                        + "in line no. "
                        + std::to_string(offendingSymbol->getLine())
                        + " near position "
                        + std::to_string(charPositionInLine);

    msg::errors::log_lexing_err(error_message, com_un::comp_units.back()->source_file);
    exit(1);
}

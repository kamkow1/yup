#include <compiler/compilation_unit.h>
#include <lexer/lexer_error_listener.h>
#include <msg/errors.h>
#include <string>

#define UNUSED_PARAM(x) (void)(x)

void yupc::LexerErrorListener::syntaxError(antlr4::Recognizer *recognizer, 
                                        antlr4::Token *offendingSymbol, 
                                        size_t line, 
                                        size_t charPositionInLine, 
                                        const std::string &msg, 
                                        std::exception_ptr e) {

    UNUSED_PARAM(recognizer);
    UNUSED_PARAM(line);
    UNUSED_PARAM(msg);
    UNUSED_PARAM(e);

    std::string error_message = "unable to recognize token \""
                        + offendingSymbol->getText() + "\" "
                        + "in line no. "
                        + std::to_string(offendingSymbol->getLine())
                        + " near position "
                        + std::to_string(charPositionInLine);

    yupc::log_lexing_err(error_message, yupc::comp_units.back()->source_file);
    exit(1);
}

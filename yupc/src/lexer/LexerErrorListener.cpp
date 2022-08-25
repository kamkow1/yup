#include "compiler/CompilationUnit.h"
#include "lexer/LexerErrorListener.h"

#include <string>

#define UNUSED(x) (void)(x)

void yupc::LexerErrorListener::syntaxError(antlr4::Recognizer *recognizer, 
                                        antlr4::Token *offendingSymbol, 
                                        size_t line, 
                                        size_t charPositionInLine, 
                                        const std::string &msg, 
                                        std::exception_ptr e) 
{
    UNUSED(recognizer);
    UNUSED(line);
    UNUSED(msg);
    UNUSED(e);

    std::string errorMessage = "unable to recognize token \""
                        + offendingSymbol->getText() + "\" "
                        + "in line no. "
                        + std::to_string(offendingSymbol->getLine())
                        + " near position "
                        + std::to_string(charPositionInLine);

    UNUSED(errorMessage);
    // TODO: Print using Logger
    exit(1);
}

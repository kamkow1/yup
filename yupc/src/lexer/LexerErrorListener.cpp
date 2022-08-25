#include <compiler/CompilationUnit.h>
#include <lexer/LexerErrorListener.h>
#include <msg/errors.h>
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

    yupc::log_lexing_err(errorMessage, yupc::CompilationUnits.back()->SourceFile);
    exit(1);
}

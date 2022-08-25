#include <compiler/CompilationUnit.h>

#include <msg/errors.h>
#include <parser/ParserErrorListener.h>
#include <string>

#define UNUSED_PARAM(x) (void)(x)

void yupc::ParserErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                        antlr4::Token *offendingSymbol,
                                        size_t line,
                                        size_t charPositionInLine,
                                        const std::string &msg,
                                        std::exception_ptr e) {

    UNUSED_PARAM(recognizer);
    UNUSED_PARAM(line);
    UNUSED_PARAM(msg);
    UNUSED_PARAM(e);
    
    std::string errorMessage = "unable to parse text \""
                        + offendingSymbol->getText() + "\" "
                        + "in line no. "
                        + std::to_string(offendingSymbol->getLine())
                        + " near position "
                        + std::to_string(charPositionInLine);

    log_parsing_err(errorMessage, yupc::CompilationUnits.back()->SourceFile);
    exit(1);
}

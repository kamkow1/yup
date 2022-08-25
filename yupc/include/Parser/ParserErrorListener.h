#ifndef YUPC_PARSER_PARSER_ERROR_LISTENER_H_
#define YUPC_PARSER_PARSER_ERROR_LISTENER_H_

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

} // namespace yupc

#endif

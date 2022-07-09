
// Generated from ./YupLexer.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"




class  YupLexer : public antlr4::Lexer {
public:
  enum {
    FUNCTION = 1, TYPE_ANNOTATION = 2, ASSIGN = 3, TERMINATOR = 4, RETURN = 5, 
    COMPILER_FLAG_SYM = 6, LPAREN = 7, RPAREN = 8, LSQBR = 9, RSQBR = 10, 
    V_STRING = 11, V_INT = 12, V_FLOAT = 13, V_BOOL = 14, V_NULL = 15, IDENTIFIER = 16, 
    WHITESPACE = 17, COMMENT = 18, LINE_COMMENT = 19
  };

  explicit YupLexer(antlr4::CharStream *input);

  ~YupLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};



// Generated from ./YupLexer.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"


namespace yupc {


class  YupLexer : public antlr4::Lexer {
public:
  enum {
    KeywordReturn = 1, KeywordConst = 2, KeywordPublic = 3, KeywordGlobal = 4, 
    KeywordFunc = 5, KeywordRef = 6, KeywordLet = 7, KeywordImport = 8, 
    KeywordType = 9, KeywordAlias = 10, SymbolAt = 11, SymbolApostrophe = 12, 
    SymbolTerminator = 13, SymbolAssign = 14, SymbolColon = 15, SymbolVariadicArgs = 16, 
    SymbolLparen = 17, SymbolRparen = 18, SymbolLbrace = 19, SymbolRbrace = 20, 
    SymbolLsqbr = 21, SymbolRsqbr = 22, SymbolComma = 23, SymbolQuestMark = 24, 
    SymbolAsterisk = 25, SymbolAmpersand = 26, SymbolDot = 27, SymbolExclMark = 28, 
    SymbolTilde = 29, SymbolArrow = 30, SymbolPlus = 31, SymbolMinus = 32, 
    SymbolSlash = 33, ValueChar = 34, ValueString = 35, ValueInteger = 36, 
    ValueFloat = 37, ValueBool = 38, ValueNull = 39, Identifier = 40, Whitespace = 41, 
    Comment = 42, MultilineComment = 43
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

}  // namespace yupc

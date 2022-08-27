
// Generated from ./YupLexer.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"


namespace yupc {


class  YupLexer : public antlr4::Lexer {
public:
  enum {
    KeywordReturn = 1, KeywordConst = 2, KeywordExport = 3, KeywordGlobal = 4, 
    KeywordRef = 5, KeywordVar = 6, KeywordImport = 7, KeywordType = 8, 
    KeywordAlias = 9, KeywordFunc = 10, SymbolAt = 11, SymbolApostrophe = 12, 
    SymbolTerminator = 13, SymbolAssign = 14, SymbolColon = 15, SymbolVariadicArgs = 16, 
    SymbolLparen = 17, SymbolRparen = 18, SymbolLbrace = 19, SymbolRbrace = 20, 
    SymbolLsqbr = 21, SymbolRsqbr = 22, SymbolComma = 23, SymbolQuestMark = 24, 
    SymbolAsterisk = 25, SymbolAmpersand = 26, SymbolDot = 27, SymbolExclMark = 28, 
    SymbolTilde = 29, SymbolArrow = 30, SymbolPlus = 31, SymbolMinus = 32, 
    SymbolSlash = 33, SymbolEqual = 34, SymbolNotEqual = 35, ValueChar = 36, 
    ValueString = 37, ValueInteger = 38, ValueFloat = 39, ValueBool = 40, 
    ValueNull = 41, Identifier = 42, Whitespace = 43, Comment = 44, MultilineComment = 45
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

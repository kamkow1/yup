
// Generated from ./YupParser.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"




class  YupParser : public antlr4::Parser {
public:
  enum {
    FUNCTION = 1, TYPE_ANNOTATION = 2, ASSIGN = 3, TERMINATOR = 4, RETURN = 5, 
    COMPILER_FLAG_SYM = 6, LPAREN = 7, RPAREN = 8, LSQBR = 9, RSQBR = 10, 
    V_STRING = 11, V_INT = 12, V_FLOAT = 13, V_BOOL = 14, V_NULL = 15, IDENTIFIER = 16, 
    WHITESPACE = 17, COMMENT = 18, LINE_COMMENT = 19
  };

  enum {
    RuleFile = 0, RuleStatement = 1, RuleCompiler_flag = 2, RuleAssignment = 3, 
    RuleFunc_def = 4, RuleFunc_signature = 5, RuleFunc_return = 6, RuleCode_block = 7, 
    RuleFunc_param = 8, RuleType_annot = 9, RuleExpr = 10, RuleConstant = 11, 
    RuleFunc_call = 12
  };

  explicit YupParser(antlr4::TokenStream *input);

  YupParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~YupParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class FileContext;
  class StatementContext;
  class Compiler_flagContext;
  class AssignmentContext;
  class Func_defContext;
  class Func_signatureContext;
  class Func_returnContext;
  class Code_blockContext;
  class Func_paramContext;
  class Type_annotContext;
  class ExprContext;
  class ConstantContext;
  class Func_callContext; 

  class  FileContext : public antlr4::ParserRuleContext {
  public:
    FileContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FileContext* file();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    antlr4::tree::TerminalNode *TERMINATOR();
    Func_defContext *func_def();
    AssignmentContext *assignment();
    Func_returnContext *func_return();
    Compiler_flagContext *compiler_flag();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  Compiler_flagContext : public antlr4::ParserRuleContext {
  public:
    Compiler_flagContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMPILER_FLAG_SYM();
    antlr4::tree::TerminalNode *IDENTIFIER();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Compiler_flagContext* compiler_flag();

  class  AssignmentContext : public antlr4::ParserRuleContext {
  public:
    AssignmentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    ExprContext *expr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentContext* assignment();

  class  Func_defContext : public antlr4::ParserRuleContext {
  public:
    Func_defContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Func_signatureContext *func_signature();
    Code_blockContext *code_block();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_defContext* func_def();

  class  Func_signatureContext : public antlr4::ParserRuleContext {
  public:
    Func_signatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    Type_annotContext *type_annot();
    std::vector<Func_paramContext *> func_param();
    Func_paramContext* func_param(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_signatureContext* func_signature();

  class  Func_returnContext : public antlr4::ParserRuleContext {
  public:
    Func_returnContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURN();
    ExprContext *expr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_returnContext* func_return();

  class  Code_blockContext : public antlr4::ParserRuleContext {
  public:
    Code_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LSQBR();
    antlr4::tree::TerminalNode *RSQBR();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Code_blockContext* code_block();

  class  Func_paramContext : public antlr4::ParserRuleContext {
  public:
    Func_paramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    Type_annotContext *type_annot();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_paramContext* func_param();

  class  Type_annotContext : public antlr4::ParserRuleContext {
  public:
    Type_annotContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TYPE_ANNOTATION();
    antlr4::tree::TerminalNode *IDENTIFIER();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Type_annotContext* type_annot();

  class  ExprContext : public antlr4::ParserRuleContext {
  public:
    ExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstantContext *constant();
    Func_callContext *func_call();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExprContext* expr();

  class  ConstantContext : public antlr4::ParserRuleContext {
  public:
    ConstantContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *V_STRING();
    antlr4::tree::TerminalNode *V_INT();
    antlr4::tree::TerminalNode *V_FLOAT();
    antlr4::tree::TerminalNode *V_BOOL();
    antlr4::tree::TerminalNode *V_NULL();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstantContext* constant();

  class  Func_callContext : public antlr4::ParserRuleContext {
  public:
    Func_callContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_callContext* func_call();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};


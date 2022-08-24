parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file                :   statement*;

statement           :   expr                    TERMINATOR
                    |   func_def
                    |   assignment              TERMINATOR
                    |   func_return             TERMINATOR
                    |   var_declare             TERMINATOR
                    |   arr_elem_assignment     TERMINATOR
                    |   func_signature          TERMINATOR
                    |   import_decl             TERMINATOR
                    |   type_decl               TERMINATOR
                    |   code_block;

type_decl           :   PUBSYM? TYPE IDENTIFIER type_def;

type_def            :   type_alias;

import_decl         :   IMPORT V_STRING;

lib_path            :   IDENTIFIER COLON;

import_list         :   LBRACE (IDENTIFIER (COMMA IDENTIFIER)*)? RBRACE;

func_import_sig     :   IDENTIFIER LPAREN (type_annot (COMMA type_annot)*)? RPAREN type_annot;

assignment          :   IDENTIFIER var_value;

arr_elem_assignment :   IDENTIFIER arr_index* var_value;

arr_index           :   LSQBR expr RSQBR;

var_declare         :   PUBSYM? GLOBAL? CONST? REF? LET IDENTIFIER type_annot var_value?;

var_value           :   ASSIGN expr;

func_def            :   func_signature code_block;

func_signature      :   PUBSYM? IDENTIFIER LPAREN (func_param (COMMA func_param)*)? RPAREN ARROW type_name;

func_return         :   RETURN expr;

code_block          :   LBRACE statement* RBRACE;

func_param          :   (IDENTIFIER type_annot ) | VAR_ARGS;

type_annot          :   COLON type_name;

type_name           :   IDENTIFIER type_ext*;

type_ext            :   array_type_ext | ASTERISK | QUEST_MARK;

array_type_ext      :   LSQBR expr RSQBR;

expr                :   constant                        #ConstantExpr
                    |   func_call                       #FuncCallExpr
                    |   IDENTIFIER                      #IdentifierExpr
                    |   array                           #ArrayExpr
                    |   array_init                      #ArrayInitExpr
                    |   addr_of                         #AddrOfExpr
                    |   expr (LSQBR expr RSQBR)+        #IndexedAccessExpr
                    |   expr binop expr                 #MathOperExpr
                    |   LPAREN expr RPAREN              #EmphExpr
                    |   ptr_dereference                 #PtrDereferenceExpr
                    |   operator                        #OperatorExpr
                    |   type_name_expr                  #TypeNameExpr
                    |   LPAREN expr RPAREN expr         #TypeCastExpr;

type_name_expr      :   APOSTROPHE type_name;

type_alias          :   ALIAS type_name;

operator            :   IDENTIFIER EXCL_MARK LPAREN expr+ RPAREN;

ptr_dereference     :   ASTERISK expr;

binop               :   PLUS | MINUS | ASTERISK | SLASH ;

addr_of             :   AMPERSAND IDENTIFIER;

array               :   LSQBR (expr (COMMA expr)*)? RSQBR;

array_init          :   LPAREN expr COMMA type_name RPAREN;

constant            :   V_STRING | V_INT | V_FLOAT | V_BOOL | V_CHAR | V_NULL;

func_call           :   IDENTIFIER LPAREN (expr (COMMA expr)*)? RPAREN; //IDENTIFIER ((LPAREN (expr (COMMA expr)*)? RPAREN) | ());

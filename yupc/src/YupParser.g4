parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file                :   statement*;

statement           :   expr                    TERMINATOR
                    |   func_def
                    |   assignment              TERMINATOR
                    |   func_return             TERMINATOR;

assignment          :   CONST? IDENTIFIER ASSIGN expr;

func_def            :   func_signature code_block;

func_signature      :   FUNCTION IDENTIFIER LPAREN (func_param (COMMA func_param)*)? RPAREN type_annot;

func_return         :   RETURN expr;

code_block          :   LBRACE statement* RBRACE;

func_param          :   IDENTIFIER type_annot;

type_annot          :   TYPE_ANNOTATION array_type? IDENTIFIER;

array_type          :   LSQBR V_INT RSQBR;

expr                :   constant                #ConstantExpr
                    |   func_call               #FuncCallExpr
                    |   IDENTIFIER              #IdentifierExpr
                    |   array                   #ArrayExpr;

array               :   LSQBR (expr (COMMA expr)*)? RSQBR type_annot;

constant            :   V_STRING | V_INT | V_FLOAT | V_BOOL | V_NULL | V_CHAR;

func_call           :   IDENTIFIER LPAREN (expr (COMMA expr)*)? RPAREN;
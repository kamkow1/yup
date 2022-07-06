parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file                :   (statement TERMINATOR)*;

statement           :   expr
                    |   func_def
                    |   assignment;

assignment          :   IDENTIFIER ASSIGN expr;

func_def            :   FUNCTION IDENTIFIER LPAREN (func_param*)? RPAREN type_annot code_block;

code_block          :   LSQBR statement* RSQBR;

func_param          :   IDENTIFIER type_annot;

type_annot          :   TYPE_ANNOTATION IDENTIFIER;

expr                :   constant
                    |   func_call;

constant            :   V_STRING | V_INT | V_FLOAT | V_BOOL | V_NULL;

func_call           :   IDENTIFIER ((expr*)? | LPAREN RPAREN);
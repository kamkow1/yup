parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file                :   statement*;

statement           :   expr                    TERMINATOR
                    |   func_def
                    |   assignment              TERMINATOR
                    |   func_return             TERMINATOR
                    |   compiler_flag;

compiler_flag       :   COMPILER_FLAG_SYM IDENTIFIER;

assignment          :   IDENTIFIER ASSIGN expr;

func_def            :   func_signature code_block;

func_signature      :   FUNCTION IDENTIFIER LPAREN (func_param*)? RPAREN type_annot;

func_return         :   RETURN expr;

code_block          :   LSQBR statement* RSQBR;

func_param          :   IDENTIFIER type_annot;

type_annot          :   TYPE_ANNOTATION IDENTIFIER;

expr                :   constant
                    |   func_call;

constant            :   V_STRING | V_INT | V_FLOAT | V_BOOL | V_NULL;

func_call           :   IDENTIFIER ((expr*)? | LPAREN RPAREN);
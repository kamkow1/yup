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
                    |   arr_elem_assignment     TERMINATOR;

assignment          :   IDENTIFIER var_value;

arr_elem_assignment :   IDENTIFIER arr_index* var_value;

arr_index           :   LSQBR expr RSQBR;

var_declare         :   CONST? LET? IDENTIFIER type_annot var_value?;

var_value           :   ASSIGN expr;

func_def            :   func_signature code_block;

func_signature      :   FUNCTION IDENTIFIER LPAREN (func_param (COMMA func_param)*)? RPAREN type_annot;

func_return         :   RETURN expr;

code_block          :   LBRACE statement* RBRACE;

func_param          :   IDENTIFIER type_annot;

type_annot          :   TYPE_ANNOTATION type_name;

type_name           :   IDENTIFIER (type_ext*)?;

type_ext            :   ASTERISK | AMPERSAND;

expr                :   constant                        #ConstantExpr
                    |   func_call                       #FuncCallExpr
                    |   IDENTIFIER                      #IdentifierExpr
                    |   array                           #ArrayExpr
                    |   addr_of                         #AddrOfExpr
                    |   expr LSQBR expr RSQBR           #IndexedAccessExpr;

addr_of             :   AMPERSAND IDENTIFIER;

array               :   LSQBR (expr (COMMA expr)*)? RSQBR;

constant            :   V_STRING | V_INT | V_FLOAT | V_BOOL | V_NULL | V_CHAR;

func_call           :   IDENTIFIER LPAREN (expr (COMMA expr)*)? RPAREN;
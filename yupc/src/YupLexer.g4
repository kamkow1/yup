lexer grammar YupLexer;

FUNCTION            :   'fun';
TYPE_ANNOTATION     :   '::';
ASSIGN              :   '=';
TERMINATOR          :   ';';
RETURN              :   'return';
COMPILER_FLAG_SYM   :   '@';

LPAREN              :   '(';
RPAREN              :   ')';
LSQBR               :   '{';
RSQBR               :   '}';

V_STRING            :   '"' (~[\\"\r\n])* '"';
V_INT               :   '-'? '0'..'9'+;
V_FLOAT             :   '-'? ('0'..'9')+ '.' ('0'..'9')*;
V_BOOL              :   ('True' | 'False');
V_NULL              :   'Null';

IDENTIFIER          :   ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_')*;
WHITESPACE          :   [ \r\n\t]+    -> skip;
COMMENT             :   '/*' .*? '*/' -> channel(HIDDEN);
LINE_COMMENT        :   '//' ~[\r\n]*  -> channel(HIDDEN);
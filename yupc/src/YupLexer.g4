lexer grammar YupLexer;

TYPE_ANNOTATION     :   ':';
ASSIGN              :   '=';
TERMINATOR          :   ';';
RETURN              :   'return';
COMPILER_FLAG_SYM   :   '@';
CONST               :   'const';
PUBSYM              :   'pubsym';
GLOBAL              :   'glob';
IMPORT              :   'import';
FROM                :   'from';
MODULE              :   'module';

LPAREN              :   '(';
RPAREN              :   ')';
LBRACE              :   '{';
RBRACE              :   '}';
LSQBR               :   '[';  
RSQBR               :   ']';
COMMA               :   ',';
QUEST_MARK          :   '?';
ASTERISK            :   '*';
AMPERSAND           :   '&';
DOT                 :   '.';

PLUS                :   '+';
MINUS               :   '-';
SLASH               :   '/';

V_CHAR              :   '\'' ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_' | '\n' | '\r') '\'';
V_STRING            :   '"' ( Escape | ~('\'' | '\\' | '\n' | '\r') )* '"';
fragment Escape     :   '\\' ( '\'' | '\\' );
V_INT               :   '-'? '0'..'9'+;
V_FLOAT             :   '-'? ('0'..'9')+ '.' ('0'..'9')*;
V_BOOL              :   ('True' | 'False');

IDENTIFIER          :   ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_')*;
WHITESPACE          :   [ \r\n\t]+    -> skip;
COMMENT             :   '/*' .*? '*/' -> channel(HIDDEN);
LINE_COMMENT        :   '//' ~[\r\n]*  -> channel(HIDDEN);
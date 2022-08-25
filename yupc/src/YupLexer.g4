lexer grammar YupLexer;

KeywordReturn               :   'return';
KeywordConst                :   'const';
KeywordPublic               :   'public';
KeywordGlobal               :   'glob';
KeywordFunc                 :   'func';
KeywordRef                  :   'ref';
KeywordLet                  :   'let';
KeywordImport               :   'import';
KeywordType                 :   'type';
KeywordAlias                :   'alias';

SymbolAt                    :   '@';
SymbolApostrophe            :   '\'';
SymbolTerminator            :   ';';
SymbolAssign                :   '=';
SymbolColon                 :   ':';
SymbolVariadicArgs          :   '...';
SymbolLparen                :   '(';
SymbolRparen                :   ')';
SymbolLbrace                :   '{';
SymbolRbrace                :   '}';
SymbolLsqbr                 :   '[';  
SymbolRsqbr                 :   ']';
SymbolComma                 :   ',';
SymbolQuestMark             :   '?';
SymbolAsterisk              :   '*';
SymbolAmpersand             :   '&';
SymbolDot                   :   '.';
SymbolExclMark              :   '!';
SymbolTilde                 :   '~';
SymbolArrow                 :   '->';
SymbolPlus                  :   '+';
SymbolMinus                 :   '-';
SymbolSlash                 :   '/';

ValueChar                   :   '\'' ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_' | '\n' | '\r') '\'';
ValueString                 :   '"' (~'"' | '\'\'')* '"';
ValueInteger                :   '-'? '0'..'9'+;
ValueFloat                  :   '-'? ('0'..'9')+ '.' ('0'..'9')*;
ValueBool                   :   ('True' | 'False');
ValueNull                   :   'Null';

Identifier                  :   ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_')*;
Whitespace                  :   [ \r\n\t]+    -> skip;
Comment                     :   '/*' .*? '*/' -> channel(HIDDEN);
MultilineComment            :   '//' ~[\r\n]*  -> channel(HIDDEN);
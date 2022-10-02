lexer grammar YupLexer;

KeywordReturn               :   'return';
KeywordConst                :   'const';
KeywordGlobal               :   'global';
KeywordVar                  :   'var';
KeywordImport               :   'import';
KeywordIf		            :	'if';
KeywordElse		    		:	'else';
KeywordFor		    		:	'for';
KeywordIn		    		:	'in';
KeywordTo		    		:	'to';
KeywordContinue		    	:	'continue';
KeywordBreak		    	:	'break';
KeywordStruct		    	:   'struct';
KeywordTypeAlias	    	:   'typealias';

SymbolArrow					:   '->';
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
SymbolPlus                  :   '+';
SymbolMinus                 :   '-';
SymbolSlash                 :   '/';
SymbolEqual                 :   '==';
SymbolNotEqual              :   '!=';
SymbolLessThan              :   '<';
SymbolMoreThan              :   '>';
SymbolLessOrEqual           :   '<=';
SymbolMoreOrEqual           :   '>=';
SymbolHash                  :   '#';
SymbolAnd                   :   'and';
SymbolOr                    :   'or';
SymbolNot                   :   'not';

ValueChar                   :   '\'' ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_' | '\n' | '\r') '\'';
fragment EscapedValue	    :   '\\' [abfnrtv\\'"];
ValueString		    		:   '"' (~["\\] | EscapedValue)* '"';
ValueRawString		    	:	'`' ~'`'* '`'; 


ValueInteger                :   '-'? '0'..'9'+;
ValueFloat                  :   '-'? ('0'..'9')+ '.' ('0'..'9')*;
ValueBool                   :   ('True' | 'False');
ValueNull                   :   'Null';

Identifier                  :   ('a'..'z' | 'A'..'Z' | '_') ('a'..'z' | 'A' .. 'Z' | '0'..'9' | '_')*;
Whitespace                  :   [ \r\n\t]+    -> skip;
Comment                     :   '/*' .*? '*/' -> channel(HIDDEN);
MultilineComment            :   '//' ~[\r\n]*  -> channel(HIDDEN);

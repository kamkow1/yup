parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file: statement*;

codeBlock: SymbolLbrace statement* SymbolRbrace;

attrList: SymbolHash SymbolLsqbr (attr (SymbolComma attr)*)? SymbolRsqbr;
attr: Identifier SymbolLparen (ValueString (SymbolComma ValueString)*)? SymbolRparen;

typeAnnot: SymbolColon typeName;
typeName: (typeExt* (Identifier | structType | funcType));
typeExt: arrayTypeExt | SymbolAsterisk | SymbolQuestMark;
arrayTypeExt: SymbolLsqbr ValueInteger SymbolRsqbr;
structType: SymbolLbrace (typeName (SymbolComma typeName)*)? SymbolRbrace;
funcType: SymbolLparen funcParamList? SymbolRparen (SymbolArrow typeName)?;

importDecl: KeywordImport ValueString+;

arrayIndex: SymbolLsqbr expression SymbolRsqbr;
constArray: SymbolLsqbr (expression (SymbolComma expression)*)? SymbolRsqbr;

assign: Identifier varValue;
varDecl: attrList? KeywordPublic? declType Identifier (SymbolComma Identifier)* typeAnnot? varValue?;
declType: (KeywordVar | KeywordConst);
varValue: SymbolAssign expression;
exprAssign: expression varValue;

funcDef: funcSig codeBlock;
funcSig: attrList? KeywordPublic? Identifier SymbolColon KeywordFunction
        SymbolLparen funcParamList? SymbolRparen (SymbolArrow typeName)?;
funcParamList: funcParam (SymbolComma funcParam)*;
funcReturn: KeywordReturn (expression (SymbolComma expression)*)?;
funcParam: (KeywordConst? Identifier typeAnnot) | SymbolVariadicArgs;
funcCall: Identifier SymbolLparen funcCallArgList? SymbolRparen;
funcCallArgList: expression (SymbolComma expression)*;

binop: SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

addressOf: SymbolAmpersand Identifier;

ifStatement: KeywordIf expression ifThenBlock ifElseBlock?;
ifThenBlock: codeBlock;
ifElseBlock: KeywordElse codeBlock;

constant: stringExpr | (ValueInteger typeAnnot?) | ValueFloat | ValueChar | ValueNull;
multilineString: ValueString+;
stringExpr: (KeywordValueString? ValueString) | multilineString;

compOper: SymbolEqual | SymbolNotEqual | SymbolMoreThan | SymbolLessThan | SymbolLessOrEqual | SymbolMoreOrEqual;

forLoopStatement: KeywordFor (arrayBasedLoop | conditionBasedLoop | statementBasedLoop) codeBlock;
arrayBasedLoop:	varDecl KeywordIn expression;
conditionBasedLoop: expression;
statementBasedLoop: statement+;
finalStatement:	statement;
continueStatement: KeywordContinue;
breakStatement: KeywordBreak;

structDeclaration: attrList? KeywordPublic? Identifier SymbolColon KeywordStruct ((SymbolLbrace structField+ SymbolRbrace) | SymbolTerminator);
structField: Identifier typeAnnot SymbolTerminator;
fieldAssignment: expression SymbolDot Identifier varValue;
typeAliasDeclaration: Identifier SymbolColon KeywordTypeAlias typeName;
structInit: Identifier SymbolDot SymbolLbrace (expression (SymbolComma expression)*)? SymbolRbrace;
constStructInit: SymbolDot SymbolLbrace (expression (SymbolComma expression)*)? SymbolRbrace;

expression: funcCall                                            #funcCallExpr
        |   Identifier                                          #identifierExpr
        |   constArray                                          #constArrayExpr
        |   addressOf                                           #addressOfExpr
        |   expression (SymbolLsqbr expression SymbolRsqbr)+    #indexedAccessExpr
        |   expression binop expression                         #binopExpr
        |   SymbolLparen expression SymbolRparen                #emphasizedExpr
        |   expression SymbolDot SymbolAsterisk                 #ptrDerefExpr
        |   SymbolLparen expression SymbolRparen expression     #typeCastExpression
        |   expression compOper expression                      #CompExpr
        |   (SymbolNot | SymbolExclMark) expression             #NegatedExpression
        |   expression SymbolAnd expression                     #LogicalAndExpression
        |   expression SymbolOr expression                      #LogicalOrExpression
        |   constant                                            #constantExpression
        |   multilineString					#MultilineStringExpression
        |   expression SymbolDot Identifier			#FieldAccessExpression
        |   expression SymbolDot funcCall                       #MethodCallExpression
        |   structInit                                          #StructInitExpression
        |   constStructInit                                     #ConstStructInitExpression
        |   typeName                                            #LitTypeExpr
        |   expression (SymbolIncrement | SymbolDecrement)	#IncremDecremExpr;

statement: expression SymbolTerminator
        |  assign SymbolTerminator
        |  exprAssign SymbolTerminator
        |  funcReturn SymbolTerminator
        |  varDecl SymbolTerminator
        |  funcSig SymbolTerminator
        |  importDecl SymbolTerminator
        |  continueStatement SymbolTerminator
        |  breakStatement SymbolTerminator
        |  typeAliasDeclaration SymbolTerminator
        |  fieldAssignment SymbolTerminator
        |  forLoopStatement
        |  ifStatement
        |  funcDef
        |  codeBlock
        |  SymbolTerminator
        |  structDeclaration;


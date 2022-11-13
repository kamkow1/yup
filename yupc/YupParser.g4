parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file: statement*;

codeBlock: SymbolLbrace statement* SymbolRbrace;

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
varDecl: KeywordExtern? KeywordPublic? declType Identifier (SymbolComma Identifier)* typeAnnot? varValue?;
declType: (KeywordVar | KeywordConst);
varValue: SymbolAssign expression;
exprAssign: expression varValue;

funcDef: funcSig codeBlock;
funcSig: KeywordExtern? KeywordPublic? Identifier SymbolColon KeywordFunction
        SymbolLparen funcParamList? SymbolRparen (SymbolArrow typeName)?;
funcParamList: funcParam (SymbolComma funcParam)*;
funcReturn: KeywordReturn (expression (SymbolComma expression)*)?;
funcParam: (KeywordConst? Identifier typeAnnot) | SymbolVariadicArgs | KeywordSelf;
funcCall: Identifier SymbolLparen funcCallArgList? SymbolRparen;
funcCallArgList: expression (SymbolComma expression)*;

binop: SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

addressOf: SymbolAmpersand Identifier;

ifStatement: KeywordIf expression ifThenBlock ifElseBlock?;
ifThenBlock: codeBlock;
ifElseBlock: KeywordElse codeBlock;

constant: stringExpr | (ValueInteger typeAnnot?) | ValueFloat | ValueChar | (ValueNull typeAnnot) | (ValueNullptr typeAnnot);
multilineString: (ValueString SymbolBackSlash)+;
stringExpr: KeywordValueString? (ValueString | multilineString);

compOper: KeywordUnsig? (SymbolEqual | SymbolNotEqual | SymbolMoreThan | SymbolLessThan | SymbolLessOrEqual | SymbolMoreOrEqual);

forLoopStatement: KeywordFor (arrayBasedLoop | conditionBasedLoop | statementBasedLoop) codeBlock;
arrayBasedLoop:	varDecl KeywordIn expression;
conditionBasedLoop: expression;
statementBasedLoop: statement+;
finalStatement:	statement;
continueStatement: KeywordContinue;
breakStatement: KeywordBreak;

structDeclaration: KeywordPublic? Identifier SymbolColon KeywordStruct 
        (SymbolExclMark Identifier (SymbolComma Identifier)*)?
		((SymbolLbrace structField* funcDef* SymbolRbrace) | SymbolTerminator);
structField: Identifier typeAnnot SymbolTerminator;
typeAliasDeclaration: KeywordPublic? Identifier SymbolColon KeywordTypeAlias typeName;
structInit: Identifier SymbolDot SymbolLbrace (expression (SymbolComma expression)*)? SymbolRbrace;
constStructInit: SymbolDot SymbolLbrace (expression (SymbolComma expression)*)? SymbolRbrace;

preprocDecl: SymbolHash Identifier Identifier? expression? SymbolTerminator?;

expression: funcCall                                            #funcCallExpr
        |   (Identifier | KeywordSelf)                          #identifierExpr
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
        |   multilineString										#MultilineStringExpression
        |   Identifier SymbolColon SymbolColon expression SymbolDot Identifier #FieldAccessExpression
        |   Identifier SymbolColon SymbolColon expression SymbolDot funcCall #MethodCallExpr
        |	Identifier SymbolColon SymbolColon funcCall			#StaticMethodCallExpr
        |   structInit                                          #StructInitExpression
        |   constStructInit                                     #ConstStructInitExpression
        |   typeName                                            #LitTypeExpr
        |   Identifier (SymbolIncrement | SymbolDecrement)	    #IncremDecremExpr
        |   preprocDecl                                         #MacroRef;

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
        |  forLoopStatement
        |  ifStatement
        |  funcDef
        |  codeBlock
        |  SymbolTerminator
        |  structDeclaration
        |  preprocDecl;


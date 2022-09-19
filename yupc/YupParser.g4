parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file:                           statement*;

codeBlock:                      SymbolLbrace statement* SymbolRbrace;

attributeList:                  SymbolHash SymbolLparen (attribute (SymbolComma attribute)?)+ SymbolRparen;
attribute:                      Identifier SymbolLparen ValueString* SymbolRparen;

typeAnnotation:                 SymbolColon typeName;
typeName:                       Identifier typeExtension*;
typeExtension:                  arrayTypeExtension | SymbolAsterisk | SymbolQuestMark;
arrayTypeExtension:             SymbolLsqbr ValueInteger SymbolRsqbr;
typeNameExpression:             SymbolApostrophe typeName;

importDeclaration:              KeywordImport ValueString;

arrayElementAssignment:         Identifier arrayIndex* variableValue;
arrayIndex:                     SymbolLsqbr expression SymbolRsqbr;
array:                          SymbolLsqbr (expression (SymbolComma expression)*)? SymbolRsqbr;

assignment:                     Identifier variableValue;
variableDeclare:                attributeList? KeywordGlobal? declarationType Identifier typeAnnotation? variableValue?;
declarationType:                (KeywordVar | KeywordConst);
variableValue:                  SymbolAssign expression;

functionDefinition:             functionSignature codeBlock;
functionSignature:              attributeList? KeywordFunc Identifier SymbolLparen functionParameterList? SymbolRparen (SymbolArrow typeName)?;
functionParameterList:          functionParameter (SymbolComma functionParameter)*;
functionReturn:                 KeywordReturn expression?;
functionParameter:              (KeywordConst? Identifier typeAnnotation ) | SymbolVariadicArgs;
functionCall:                   Identifier SymbolLparen functionCallArgList? SymbolRparen;
functionCallArgList:            expression (SymbolComma expression)*;

binaryOperator:                 SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

yupFunction:                    SymbolAt functionCall;

pointerDereference:             SymbolAsterisk expression;
addressOf:                      SymbolAmpersand Identifier;

ifStatement:		        	KeywordIf expression ifThenBlock ifElseBlock?;
ifThenBlock:		        	codeBlock;
ifElseBlock:		        	KeywordElse codeBlock;

constant:                       ValueString | ValueInteger | ValueFloat | ValueBool | ValueChar | ValueNull;

comparisonOperator:             SymbolEqual | SymbolNotEqual | SymbolMoreThan | SymbolLessThan | SymbolLessOrEqual | SymbolMoreOrEqual;

forLoopStatement:				KeywordFor (arrayBasedLoop | conditionBasedLoop) codeBlock;
arrayBasedLoop:					variableDeclare KeywordIn expression;
conditionBasedLoop:				expression;
continueStatement:				KeywordContinue;
breakStatement:					KeywordBreak;

expression:                     functionCall                                        #functionCallExpression
        |                       Identifier                                          #identifierExpression
        |                       array                                               #arrayExpression
        |                       addressOf                                           #addressOfExpression
        |                       expression (SymbolLsqbr expression SymbolRsqbr)+    #indexedAccessExpression
        |                       expression binaryOperator expression                #binaryOperationExpression
        |                       SymbolLparen expression SymbolRparen                #emphasizedExpression
        |                       pointerDereference                                  #pointerDereferenceExpression
        |                       yupFunction                                         #yupFunctionExpression
        |                       typeNameExpression                                  #typeExpression
        |                       SymbolLparen expression SymbolRparen expression     #typeCastExpression
        |                       expression comparisonOperator expression            #ComparisonExpression
        |                       (SymbolNot | SymbolExclMark) expression             #NegatedExpression
        |                       expression SymbolAnd expression                     #LogicalAndExpression
        |                       expression SymbolOr expression                      #LogicalOrExpression
        |						SymbolExplicitIdent Identifier						#ExplicitIdentifierExpression
        |						SymbolExplicitIdent ValueString						#ExplicitStringExpression
        |						KeywordRange ValueInteger KeywordTo ValueInteger	#RangeExpression
        |						constant                                            #constantExpression;

statement:                      expression                                          SymbolTerminator
        |                       assignment                                          SymbolTerminator
        |                       functionReturn                                      SymbolTerminator
        |                       variableDeclare                                     SymbolTerminator
        |                       arrayElementAssignment                              SymbolTerminator
        |                       functionSignature                                   SymbolTerminator
        |                       importDeclaration                                   SymbolTerminator
        |						continueStatement									SymbolTerminator
        |						breakStatement										SymbolTerminator
        |						forLoopStatement
        |						ifStatement
        |                       functionDefinition
        |                       codeBlock
        |                       SymbolTerminator;

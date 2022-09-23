parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file:                           statement*;

codeBlock:                      SymbolLbrace statement* SymbolRbrace;

attributeList:                  SymbolHash SymbolLsqbr (attribute (SymbolComma attribute)?)+ SymbolRsqbr;
attribute:                      Identifier SymbolLparen ValueString* SymbolRparen;

typeAnnotation:                 SymbolColon typeName;
typeName:                       typeExtension* Identifier;
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
functionSignature:              attributeList? Identifier SymbolLparen functionParameterList? SymbolRparen typeName?;
functionParameterList:          functionParameter (SymbolComma functionParameter)*;
functionReturn:                 KeywordReturn expression?;
functionParameter:              (KeywordConst? Identifier typeAnnotation) | SymbolVariadicArgs;
functionCall:                   Identifier SymbolLparen functionCallArgList? SymbolRparen;
functionCallArgList:            expression (SymbolComma expression)*;

binaryOperator:                 SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

pointerDereference:             SymbolAsterisk expression;
addressOf:                      SymbolAmpersand Identifier;

ifStatement:		        	KeywordIf expression ifThenBlock ifElseBlock?;
ifThenBlock:		        	codeBlock;
ifElseBlock:		        	KeywordElse codeBlock;

constant:                       ValueString | ValueInteger | ValueFloat | ValueBool | ValueChar | ValueNull;
literalConstantInt:				SymbolApostrophe ValueInteger;

comparisonOperator:             SymbolEqual | SymbolNotEqual | SymbolMoreThan | SymbolLessThan | SymbolLessOrEqual | SymbolMoreOrEqual;

forLoopStatement:				KeywordFor (arrayBasedLoop | conditionBasedLoop | statementBasedLoop) codeBlock;
arrayBasedLoop:					variableDeclare KeywordIn expression;
conditionBasedLoop:				expression;
statementBasedLoop:				statement+;
finalStatement:					statement;
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
        |                       typeNameExpression                                  #typeExpression
        |                       SymbolLparen expression SymbolRparen expression     #typeCastExpression
        |                       expression comparisonOperator expression            #ComparisonExpression
        |                       (SymbolNot | SymbolExclMark) expression             #NegatedExpression
        |                       expression SymbolAnd expression                     #LogicalAndExpression
        |                       expression SymbolOr expression                      #LogicalOrExpression
        |						constant                                            #constantExpression
        |						literalConstantInt		 	  	        			#literalConstantIntExpression;

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

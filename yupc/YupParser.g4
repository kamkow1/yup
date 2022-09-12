parser grammar YupParser;

options {
    tokenVocab = 'lexer/YupLexer';
}

file:                           statement*;

codeBlock:                      SymbolLbrace statement* SymbolRbrace;

typeDeclaration:                KeywordType Identifier typeDefinition;
typeDefinition:                 typeAlias;
typeAlias:                      KeywordAlias typeName;
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
variableDeclare:                KeywordGlobal? declarationType Identifier typeAnnotation? variableValue?;
declarationType:                (KeywordVar | KeywordConst);
variableValue:                  SymbolAssign expression;

functionDefinition:             functionSignature codeBlock;
functionSignature:              KeywordFunc Identifier SymbolLparen functionParameterList? SymbolRparen SymbolArrow typeName;
functionParameterList:          functionParameter (SymbolComma functionParameter)*;
functionReturn:                 KeywordReturn expression?;
functionParameter:              (KeywordConst? Identifier typeAnnotation ) | SymbolVariadicArgs;
functionCall:                   Identifier SymbolLparen functionCallArgList? SymbolRparen;
functionCallArgList:            expression (SymbolComma expression)*;

binaryOperator:                 SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

yupFunction:                    SymbolAt functionCall;

pointerDereference:             SymbolAsterisk expression;
addressOf:                      SymbolAmpersand Identifier;

ifStatement:					KeywordIf expression ifThenBlock ifElseBlock?;
ifThenBlock:					codeBlock;
ifElseBlock:					KeywordElse codeBlock;

stringPrefix:                   KeywordGStrPrefix | KeywordLocalStrPrefix;
constant:                       (stringPrefix* ValueString) | ValueInteger | ValueFloat | ValueBool | ValueChar | ValueNull;

comparisonOperator:             SymbolEqual | SymbolNotEqual | SymbolMoreThan | SymbolLessThan | SymbolLessOrEqual | SymbolMoreOrEqual;

expression:                     constant                                            #constantExpression
        |                       functionCall                                        #functionCallExpression
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
        |                       SymbolNot expression                                #NegatedExpression;

statement:                      expression                                          SymbolTerminator
        |                       assignment                                          SymbolTerminator
        |                       functionReturn                                      SymbolTerminator
        |                       variableDeclare                                     SymbolTerminator
        |                       arrayElementAssignment                              SymbolTerminator
        |                       functionSignature                                   SymbolTerminator
        |                       importDeclaration                                   SymbolTerminator
        |                       typeDeclaration                                     SymbolTerminator
        |			ifStatement
        |                       functionDefinition
        |                       codeBlock
        |                       SymbolTerminator;

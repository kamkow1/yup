parser grammar YupParser;

options {
    tokenVocab = 'Lexer/YupLexer';
}

file:                           statement*;

codeBlock:                      SymbolLbrace statement* SymbolRbrace;

typeDeclaration:                KeywordExport? KeywordType Identifier typeDefinition;
typeDefinition:                 typeAlias;
typeAlias:                      KeywordAlias typeName;
typeAnnotation:                 SymbolColon typeName;
typeName:                       Identifier typeExtension*;
typeExtension:                  arrayTypeExtension | SymbolAsterisk | SymbolQuestMark;
arrayTypeExtension:             SymbolLsqbr expression SymbolRsqbr;
typeNameExpression:             SymbolApostrophe typeName;

importDeclaration:              KeywordImport ValueString;

arrayElementAssignment:         Identifier arrayIndex* variableValue;
arrayIndex:                     SymbolLsqbr expression SymbolRsqbr;
array:                          SymbolLsqbr (expression (SymbolComma expression)*)? SymbolRsqbr;

assignment:                     Identifier variableValue;
variableDeclare:                KeywordExport? KeywordGlobal? declarationType KeywordRef? Identifier typeAnnotation variableValue?;
declarationType:                (KeywordVar | KeywordConst);
variableValue:                  SymbolAssign expression;

functionDefinition:             functionSignature codeBlock;
functionSignature:              KeywordExport? KeywordFunc Identifier SymbolLparen functionParameterList? SymbolRparen SymbolArrow typeName;
functionParameterList:          functionParameter (SymbolComma functionParameter)*;
functionReturn:                 KeywordReturn expression;
functionParameter:              (Identifier typeAnnotation ) | SymbolVariadicArgs;
functionCall:                   Identifier SymbolLparen functionCallArgList? SymbolRparen;
functionCallArgList:            expression (SymbolComma expression)*;

binaryOperator:                 SymbolPlus | SymbolMinus | SymbolAsterisk | SymbolSlash;

operator:                       Identifier SymbolExclMark SymbolLparen expression+ SymbolRparen;

pointerDereference:             SymbolAsterisk expression;
addressOf:                      SymbolAmpersand Identifier;

constant:                       ValueString | ValueInteger | ValueFloat | ValueBool | ValueChar | ValueNull;

expression:                     constant                                            #constantExpression
        |                       functionCall                                        #functionCallExpression
        |                       Identifier                                          #identifierExpression
        |                       array                                               #arrayExpression
        |                       addressOf                                           #addressOfExpression
        |                       expression (SymbolLsqbr expression SymbolRsqbr)+    #indexedAccessExpression
        |                       expression binaryOperator expression                #binaryOperationExpression
        |                       SymbolLparen expression SymbolRparen                #emphasizedExpression
        |                       pointerDereference                                  #pointerDereferenceExpression
        |                       operator                                            #operatorExpression
        |                       typeNameExpression                                  #typeExpression
        |                       SymbolLparen expression SymbolRparen expression     #typeCastExpression;

statement:                      expression                                          SymbolTerminator
        |                       assignment                                          SymbolTerminator
        |                       functionReturn                                      SymbolTerminator
        |                       variableDeclare                                     SymbolTerminator
        |                       arrayElementAssignment                              SymbolTerminator
        |                       functionSignature                                   SymbolTerminator
        |                       importDeclaration                                   SymbolTerminator
        |                       typeDeclaration                                     SymbolTerminator
        |                       functionDefinition
        |                       codeBlock;

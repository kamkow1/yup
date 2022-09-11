#!/usr/bin/bash

readonly jarp="/usr/local/bin/antlr-4.10.1-complete.jar"

pos_args=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jar)
            antlr_jar_path=$2
            shift
            shift
        ;;
        -*|--*)
            echo "unknown option $1"
            exit 1
            ;;
        *)
        pos_args+=("$1")
        shift
        ;;
    esac
done

set -- "${pos_args[@]}"

if [[ "$antlr_jar_path" = "" ]]; then
    antlr_jar_path=$jarp
fi

echo "$antlr_jar_path"

cd src/

# clean up

rm -f -- Parser/YupParser.cpp
rm -f -- Parser/YupParserBaseVisitor.cpp
rm -f -- Parser/YupParserVisitor.cpp
rm -f -- Parser/YupParser.tokens
rm -f -- Parser/YupParser.interp

rm -f -- Lexer/YupLexer.cpp
rm -f -- Lexer/YupLexer.tokens
rm -f -- Lexer/YupLexer.interp

java -jar "$antlr_jar_path" ./YupLexer.g4 -Dlanguage=Cpp -o ./Lexer -package yupc
java -jar "$antlr_jar_path" ./YupParser.g4 -Dlanguage=Cpp -o ./Parser -no-listener -visitor -package yupc

cd ..

if [[ ! -d include/Lexer ]]; then
    mkdir include/Lexer
fi

if [[ ! -d include/Parser ]]; then
    mkdir include/Parser
fi

mv src/Lexer/YupLexer.h include/Lexer/YupLexer.h

mv src/Parser/YupParser.h include/Parser/YupParser.h
mv src/Parser/YupParserBaseVisitor.h include/Parser/YupParserBaseVisitor.h
mv src/Parser/YupParserVisitor.h include/Parser/YupParserVisitor.h

# fix includes
sed -i '5 c \#include "Parser/YupParserVisitor.h"' src/Parser/YupParser.cpp
sed -i '7 c \#include "Parser/YupParser.h"' src/Parser/YupParser.cpp
sed -i '5 c \#include "Parser/YupParserBaseVisitor.h"' src/Parser/YupParserBaseVisitor.cpp
sed -i '5 c \#include "Parser/YupParserVisitor.h"' src/Parser/YupParserVisitor.cpp

sed -i '5 c \#include "Lexer/YupLexer.h"' src/Lexer/YupLexer.cpp

sed -i '8 c \#include "Parser/YupParserVisitor.h"' include/Parser/YupParserBaseVisitor.h

echo "finished generating lexer & parser"

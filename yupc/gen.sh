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

rm -f -- parser/YupParser.cpp
rm -f -- parser/YupParserBaseVisitor.cpp
rm -f -- parser/YupParserVisitor.cpp
rm -f -- parser/YupParser.tokens
rm -f -- parser/YupParser.interp

rm -f -- lexer/YupLexer.cpp
rm -f -- lexer/YupLexer.tokens
rm -f -- lexer/YupLexer.interp

java -jar "$antlr_jar_path" ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer -package yupc
java -jar "$antlr_jar_path" ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor -package yupc

cd ..

if [[ ! -d include/lexer ]]; then
    mkdir include/lexer
fi

if [[ ! -d include/parser ]]; then
    mkdir include/parser
fi

mv src/lexer/YupLexer.h include/lexer/YupLexer.h

mv src/parser/YupParser.h include/parser/YupParser.h
mv src/parser/YupParserBaseVisitor.h include/parser/YupParserBaseVisitor.h
mv src/parser/YupParserVisitor.h include/parser/YupParserVisitor.h

# fix includes
sed -i '5 c \#include "parser/YupParserVisitor.h"' src/parser/YupParser.cpp
sed -i '7 c \#include "parser/YupParser.h"' src/parser/YupParser.cpp
sed -i '5 c \#include "parser/YupParserBaseVisitor.h"' src/parser/YupParserBaseVisitor.cpp
sed -i '5 c \#include "parser/YupParserVisitor.h"' src/parser/YupParserVisitor.cpp

sed -i '5 c \#include "lexer/YupLexer.h"' src/lexer/YupLexer.cpp

sed -i '8 c \#include "parser/YupParserVisitor.h"' include/parser/YupParserBaseVisitor.h

echo "finished generating lexer & parser"

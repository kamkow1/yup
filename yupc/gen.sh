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

java -jar "$antlr_jar_path" ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer -package YupCompiler::Lexer
java -jar "$antlr_jar_path" ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor -package YupCompiler::Parser

mv lexer/YupLexer.cpp lexer/YupLexer.cc

mv parser/YupParser.cpp parser/YupParser.cc
mv parser/YupParserBaseVisitor.cpp parser/YupParserBaseVisitor.cc
mv parser/YupParserVisitor.cpp parser/YupParserVisitor.cc

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
sed -i '5 c \#include "parser/YupParserVisitor.h"' src/parser/YupParser.cc
sed -i '7 c \#include "parser/YupParser.h"' src/parser/YupParser.cc
sed -i '5 c \#include "parser/YupParserBaseVisitor.h"' src/parser/YupParserBaseVisitor.cc
sed -i '5 c \#include "parser/YupParserVisitor.h"' src/parser/YupParserVisitor.cc

sed -i '5 c \#include "lexer/YupLexer.h"' src/lexer/YupLexer.cc

echo "finished generating lexer & parser"

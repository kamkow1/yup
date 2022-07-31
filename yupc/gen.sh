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

java -jar "$antlr_jar_path" ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer
java -jar "$antlr_jar_path" ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor

mv lexer/YupLexer.cpp lexer/YupLexer.cc

mv parser/YupParser.cpp parser/YupParser.cc
mv parser/YupParserBaseVisitor.cpp parser/YupParserBaseVisitor.cc
mv parser/YupParserVisitor.cpp parser/YupParserVisitor.cc

cd ..

mv src/lexer/YupLexer.h include/YupLexer.h
mv src/parser/YupParser.h include/YupParser.h

echo "finished generating lexer & parser"

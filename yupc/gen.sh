#!/bin/sh
set -xe
antlr="deps/antlr4-4.11.2-SNAPSHOT-complete.jar"

if [ ! -d lexer ]; then 
	mkdir lexer
fi

java -jar "$antlr" YupLexer.g4 -Dlanguage=Go -o lexer -package lexer
mv lexer/yup_lexer.go lexer/root.go

sed -i '10 c \"github.com/antlr/antlr4/runtime/Go/antlr/v4\"' lexer/root.go

if [ ! -d parser ]; then
	mkdir parser
fi

java -jar "$antlr" YupParser.g4 -Dlanguage=Go -o parser -package parser -no-listener -visitor
mv parser/yup_parser.go parser/root.go

sed -i '9 c \"github.com/antlr/antlr4/runtime/Go/antlr/v4\"' parser/root.go

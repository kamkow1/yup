#!/bin/sh
set -xe
antlr="deps/antlr4-4.10.2-SNAPSHOT-complete.jar"

if [ ! -d lexer ]; then 
	mkdir lexer
fi

java -jar "$antlr" YupLexer.g4 -Dlanguage=Go -o lexer -package lexer
mv lexer/yup_lexer.go lexer/root.go

if [ ! -d parser ]; then
	mkdir parser
fi

java -jar "$antlr" YupParser.g4 -Dlanguage=Go -o parser -package parser -no-listener -visitor
mv parser/yup_parser.go parser/root.go

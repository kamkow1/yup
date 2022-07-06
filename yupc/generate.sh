jar_loc="/usr/local/bin/antlr-4.10.1-complete.jar"

cd src/

java -jar $jar_loc ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer

java -jar $jar_loc ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor

mv lexer/YupLexer.cpp lexer/YupLexer.cc

mv parser/YupParser.cpp parser/YupParser.cc

# clean up

if [ -d .antlr/ ]; then
    rm -r .antlr/
fi
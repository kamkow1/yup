jar_loc="/usr/local/bin/antlr-4.10.1-complete.jar"

cd src/

java -jar $jar_loc ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer

mv lexer/YupLexer.cpp lexer/YupLexer.cc

# clean up

if [ -d .antlr/ ]; then
    rm -r .antlr/
fi
jar_loc="/usr/local/bin/antlr-4.10.1-complete.jar"

cd src/

java -jar $jar_loc ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer

java -jar $jar_loc ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor

# rename
mv lexer/YupLexer.cpp lexer/YupLexer.cc
mv parser/YupParser.cpp parser/YupParser.cc

cd ..

mv src/lexer/YupLexer.h include/YupLexer.h
mv src/parser/YupParser.h include/YupParser.h
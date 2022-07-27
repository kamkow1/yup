from asyncio.subprocess import PIPE
from cgi import print_form
import sys
import os
import subprocess
import argparse

# USAGE: python3 ./generate.py
# with custom antlr4 location: python3 ./generate.py -j /my/path/to/antlr4.jar

parser = argparse.ArgumentParser(
    description="ANTLR4 grammar generator")

parser.add_help = True

parser.add_argument("-j", "--jar",
    required=False,
    help="overrides the antlr4 jar path")

args = parser.parse_args()

antlr_jar_path = "/usr/local/bin/antlr-4.10.1-complete.jar"

if (args.jar is not None):
    print(f"setting custom antlr4 jar path: {args.jar}")
    antlr_jar_path = args.jar

os.chdir("src") # change dir to src/

lexer_gen_cmd = f"java -jar {antlr_jar_path} ./YupLexer.g4 -Dlanguage=Cpp -o ./lexer"
proc = subprocess.Popen(lexer_gen_cmd, 
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(text)

proc.wait()

if (proc.returncode != 0):
    print(f"failed to generate lexer files ({lexer_gen_cmd}). error code: {proc.returncode}")
    sys.exit(1)
else:
    print(f"successfully generated lexer files ({lexer_gen_cmd})")

del proc

rename_lexer_cmd = "mv lexer/YupLexer.cpp lexer/YupLexer.cc"
proc = subprocess.Popen(rename_lexer_cmd,
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(text)

proc.wait()

if (proc.returncode != 0):
    print("failed to rename lexer source file ( .cpp => .cc )")
    sys.exit(1)
else:
    print("successfully renamed lexer source file ( .cpp => .cc )")

del proc

parser_gen_cmd = f"java -jar {antlr_jar_path} ./YupParser.g4 -Dlanguage=Cpp -o ./parser -no-listener -visitor"
proc = subprocess.Popen(parser_gen_cmd,
    shell=True, stdout=subprocess.PIPE)

for line in proc.stdout:
    text = line.decode()
    print(line)

proc.wait()

if (proc.returncode != 0):
    print(f"failed to generate parser files ({parser_gen_cmd}). error code: {proc.returncode}")
    sys.exit(1)
else:
    print(f"successfully generated parser files ({parser_gen_cmd})")

del proc

parser_rename_cmds = [
    "mv parser/YupParser.cpp parser/YupParser.cc",
    "mv parser/YupParserBaseVisitor.cpp parser/YupParserBaseVisitor.cc",
    "mv parser/YupParserVisitor.cpp parser/YupParserVisitor.cc"
]

for prc in parser_rename_cmds:
    proc = subprocess.Popen(prc, 
        shell=True, stdout=PIPE)
    
    for line in proc.stdout:
        text = line.decode()
        print(text)

    proc.wait()

    if (proc.returncode != 0):
        print(f"failed to rename parser source file ({prc}). error code: {proc.returncode}")
        sys.exit(1)
    else:
        print(f"successfully renamed parser source file ({prc})")

    del proc

os.chdir("..") # cd to root dir

mv_h_cmds = [
    "mv src/lexer/YupLexer.h include/YupLexer.h",
    "mv src/parser/YupParser.h include/YupParser.h"
]

for mv_h in mv_h_cmds:
    proc = subprocess.Popen(mv_h,
        shell=True, stdout=PIPE)

    for line in proc.stdout:
        text = line.decode()
        print(text)

    proc.wait()

    if (proc.returncode != 0):
        print(f"failed to move header file ({mv_h})")
        sys.exit(1)
    else:
        print(f"successfully moved header file ({mv_h})")

    del proc

print("finished generating antlr4 source files")

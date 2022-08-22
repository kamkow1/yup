# yup language

## introduction

Yup is an imperative programming language. It borrows syntax from
Go, Rust and C. <br>
It's compiler is implemented using LLVM.

# installing the standard library

run:
```bash
cd /root/of/project/yup
chmod +x ./install_stdlib.sh
./install_stdlib.sh
```

now you can refer to the stdlib in your code using "```@std```" <br>
small example:

```
import "@std/memory/memlib_wrap.yup";

func main(): i32  
{
    let a: *i8 = __malloc(('i64) 8);

    __free(a);

    return 0;
}
```

# running tests
requires Python 3.10 or newer

run the tests:
```bash
cd tests
python3 tests.py
```

tests are to prevent regressions in the compiler's development

# building from source

As of now, Yup doesn't provide prebuilt binaries so you will 
have to build the compiler from source. <br>
Also, the compiler only supports Linux, but that is going to change in the future.

Before you compile yupc, you need to build all of the dependencies:
##### (you will need cmake for this)

#### 1. LLVM
Read LLVM instructions: <br>
https://www.llvm.org/docs/CMake.html

#### 2. ANTLR4
Read ANTLR4 C++ runtime instructions: <br>
https://github.com/antlr/antlr4/blob/master/runtime/Cpp/README.md

the ANTLR4 runtime can be found in the third_party directory.

In order to build the Yup compiler from source run:
```bash
git clone https://github.com/kamkow1/yup.git

cd /path/to/yup/source/yup

cd yupc

./gen.sh -j /path/to/antlr4tool.jar # generate lexer and parser source files

./build.sh -m release # -m [relrease | debug]
```

#### 3. Conan dependencies

the rest of the dependencies are provided by Conan
Learn more on how to install conan: <br>
 https://conan.io/downloads.html

# scripts

## build.sh
description: <br>
builds yupc from source (not the dependecies)

arguments: <br>
*   -m | --mode [build mode]
    - [release | debug]

## gen.sh
description: <br>
generates ANTLR4 source files (lexer and parser)

arguments: <br>
*   -j | --jar [path]
    - path: path to antlr4 tool (a .jar file)

### development roadmap
* basics [&check;]
    - [&check;] variable assignment
    - [&check;] overriding variables
    - [&check;] function declarations
    - [&check;] function calls
    - [&check;] basic data types
        * [&check;] char type
        * [&check;] 32 bit int type
        * [&check;] 64 bit int type
        * [&check;] boolean type
        * [&check;] float type
        * [&check;] null
        * [&check;] array type
        * [&check;] string type
    - [&check;] global variables
    - [&check;] external symbols
    - [&#10799;] for loop
    - [&#10799;] while loop
    - [&#10799;] conditional statements ( if + elif + else )
    - [&check;] basic module system
        * [&check;] exports
        * [&check;] imports
* OOP [&#10799;]
    - [&#10799;] structs
    - [&#10799;] access modifiers
* standard libary [&#10799;]
    - [&#10799;] memory 
        * [&check;] __malloc()
        * [&check;] __free()
* meta programming [&#10799;]
    - [&#10799;] interpreted blocks
    - [&#10799;] generics
    - [&#10799;] reflection
    - [&#10799;] built in yup interpreter

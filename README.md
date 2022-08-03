# yup language

## introduction

Yup is an imperative programming language. It borrows syntax from
Go, Rust and C. <br>
It's compiler is implemented using LLVM.

# building from source

As of now, Yup doesn't provide prebuilt binaries so you will 
have to build the compiler from source. <br>
Also, the compiler only supports Linux, but that is going to change in the future.

Before you compile yupc, you need to build all of the dependencies:
##### (you will need cmake for this)

#### 1. LLVM
```bash
cd /path/to/yup/source/yup

cd yupc/third_party

cd llvm

cd build

cmake ..

cmake --target install
```

warning: building LLVM may take up to 5+ hours!

#### 2. ANTLR4
```bash
cd /path/to/yup/source/yup

cd yupc/third_party

cd antlr4-runtime

mkdir build && mkdir run && cd build

cmake ..

make
```

In order to build the Yup compiler from source run:
```bash
git clone https://github.com/kamkow1/yup.git

cd /path/to/yup/source/yup

cd yupc

./gen.sh -j /path/to/antlr4tool.jar # generate lexer and parser source files

./build.sh -m release # -m [relrease | debug]
```

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

## run.sh
description: <br>
allows for quick runs (instead of typing complex yupc's cli commands) <br>
this is helpful during development to quickly run the compiler

arguments: <br>
*   r [emit LLVM IR] (--emit-ir | --ir)
    - [true | false]
*   v [verbose output] (--verbose | -v)
    - [true | false]
*   o [output object file] (--object | -o)
    - [true | false]

### development goals
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
    - [&#10799;] external symbols
    - [&#10799;] for loop
    - [&#10799;] while loop
    - [&#10799;] conditional statements ( if + elif + else )
    - [&#10799;] basic module system
        * [&check;] exports
        * [&#10799;] imports
* OOP [&#10799;]
    - [&#10799;] structs
    - [&#10799;] access modifiers
* standard libary [&#10799;]
* meta programming [&#10799;]
    - [&#10799;] interpreted blocks
    - [&#10799;] generics
    - [&#10799;] reflection
    - [&#10799;] built in yup interpreter

## example code snippet

```text

glob array_size: i32 = .i32;

fnc set_array_size(s: i32): void
    array_size = s;

fnc get_array_size(): i32
    return array_size;

fnc main(argc: i32, argv: **char): i32
{
    set_array_size(100);

    my_array: *i32 = (get_array_size(), i32);

    return 0;
}

```
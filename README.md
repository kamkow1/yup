# yup language

## UPDATE: 

the yup compiler has officially been migrated from
C++ to Go. The old C++ version is still kept around
at https://github.com/kamkow1/old-cpp-yupc.git but is no longer maintained.

## introduction

Yup is an imperative programming language. It borrows syntax from
Go, Rust and C. <br>
It's compiler is implemented using LLVM.

Small example of the language:

```text
import "$std/io.bc";

#(GC("default"))
func fac(n: i8) -> i8 {
	if n == 0 {
		return 1;
	} 

	return n * fac(n - 1);
}

#(GC("default"))
func main(argc: i32, argv: i8**) -> i8 {

	Printf(g"  %i  ", fac(5)); // prints 120 (5! = 120)

	return 0;
}
```

# installing the standard library

run:
```bash
cd /root/of/project/yup/yup_stdlib
chmod +x ./install_stdlib.sh
./build.sh
./install_stdlib.sh # installs in ~/yup_stdlib
```

# building from source

As of now, Yup doesn't provide prebuilt binaries so you will 
have to build the compiler from source. <br>
Also, the compiler only supports Linux, but that is going to change in the future. I do not own a copy of Windows, so if you do, feel free to create a port of the compiler.

```bash
cd /root/of/project/yup/yupc
./gen.sh # generates needed ANTLR4 files
./build.sh # outputs the yupc binary into bin/
```

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
    - [&#10799;] for loop
    - [&#10799;] while loop
    - [&#10799;] conditional statements ( if + elif + else )
    - [&check;] basic module system
        * [&check;] exports
        * [&check;] imports
* OOP [&#10799;]
    - [&#10799;] structs
    - [&#10799;] access modifiers
* meta programming [&#10799;]
    - [&#10799;] interpreted blocks
    - [&#10799;] generics
    - [&#10799;] reflection
    - [&#10799;] built in yup interpreter

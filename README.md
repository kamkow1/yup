# yup language

![Github Actions](https://github.com/kamkow1/yup/actions/workflows/ci.yml/badge.svg)

## introduction

Yup is an imperative programming language. It borrows syntax from
Go, Rust and C. <br>
It's compiler is implemented using LLVM.

Small example of the language:

```c
// calculate a fibonacci number for N:

import #std/IO.yup";

calc_fib: fnc(n: i64) -> i64 {
	if n < 0 {
		printf("cannot calculate a fibonnaci number for N < 0\n");
		return -1;
	}
	
	if n == 0 {
		return n;
	}
	
	if n == 1 or n == 2 {
		return 1;
	}
	
	return calc_fib(n - 1) + calc_fib(c - 2);
}

pub main: fnc() -> i32 {
	printf("give me a number N: ");
	var count: i64;
	scanf("%d", &count);
	
	printf("your number is: %d\n", calc_fib(number));
	
	return 0;
}
```

if you'd like to see a more complex usage of the language, see yup_stdlib/Vector.yup
or yup_stdlib/Core/SharedPointer.yup. these are some of the more advanced usages of Yup.

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

sudo ./llvm.sh 14 # installes LLVM 14.0.6 which is required to build the compiler

./build.sh # outputs the yupc binary into bin/
```


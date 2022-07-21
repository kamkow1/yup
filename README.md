# yup programming language
## what is "yup"?
yup is a "challange" programming language - the goal is to implement a c/rust-like language.
The yup compiler is developed using LLVM as backend.
## goal checklist
* basics :x:
    - :heavy_check_mark: variable assignment
    - :heavy_check_mark: overriding variables
    - :heavy_check_mark: function declarations
    - :heavy_check_mark: function calls
    - :x: basic data types
        * :heavy_check_mark: char type
        * :heavy_check_mark: 32 bit int type
        * :heavy_check_mark: 64 bit int type
        * :heavy_check_mark: boolean type
        * :heavy_check_mark: float type
        * :x: null
        * :x: array type
        * :x: string type
    - :x: for loop
    - :x: while loop
    - :x: conditional statements ( if + elif + else )
* OOP :x:
    - :x: structs
    - :x: classes
    - :x: methods
    - :x: fields
    - :x: encapsulation
* meta programming :x:
    - :x: interpreted blocks
    - :x: generics
    - :x: reflection
    - :x: built in yup interpreter

## example code snippet

```text
fun hello(a :: i32) :: i32 {
    return 74;
}

fun getChar(c :: char) :: char {
    const let res = c;
    return res;
}

fun main() :: void {
    let x = 1;
    let y = x;

    y = 2;

    let a = y;

    let b = hello(a);

    let c = getChar('s');
}
```
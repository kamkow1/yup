clang -opaque-pointers=0 -O0 -emit-llvm io.c -c -o io.bc
clang -opaque-pointers=0 -O0 -emit-llvm memory.c -c -o memory.bc
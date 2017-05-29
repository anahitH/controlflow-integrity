#!/bin/bash

# $1 source file 
# $2 rtlib bitcode

clang++ -std=c++0x -stdlib=libc++ $1 -c -emit-llvm -o bitcode.bc
llvm-link bitcode.bc $2 -o out.bc
opt -load ../build/lib/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
clang++ -lncurses -rdynamic -std=c++0x -stdlib=libc++ out.bc -o out
#./out


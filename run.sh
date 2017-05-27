#!/bin/bash

# $1 source file name (without extensions)
# $2 rtlib bitcode name

clang++ -std=c++0x -stdlib=libc++ $1.cpp -c -emit-llvm
llvm-link $1.bc $2.bc -o out.bc
opt -load ../build/lib/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
clang++ -rdynamic -std=c++0x -stdlib=libc++ out.bc -o out
./out


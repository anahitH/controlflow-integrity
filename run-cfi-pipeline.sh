#!/bin/bash

# $1 source file 
# $2 rtlib bitcode

clang-3.9 -std=c++11 -c -emit-llvm checkers/checker.cpp
clang-3.9 -c -emit-llvm -W -Wall -Werror -DVERSION=\"1.0.1\" -o snake.bc snake.c
llvm-link-3.9 protected.bc checkers/checker.bc -o out.bc
opt-3.9 -load build/lib/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
#clang++-3.9 -lunwind -lunwind-x86_64 out.bc -o out
llc-3.9 -filetype=obj out.bc
g++ out.o -o out -lunwind-x86_64 -lunwind #-L/usr/lib/x86_64-linux-gnu -Wl,-rpath=/usr/lib/x86_64-linux-gnu -lunwind-x86_64 -lunwind  out.o -o out

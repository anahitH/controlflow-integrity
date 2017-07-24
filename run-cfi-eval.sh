#!/bin/bash
set -e

# $1 source file 
# $2 rtlib bitcode
protected=protected.bc

bitcode=$1

clang-3.9 -std=c++11 -c -emit-llvm checkers/checker.cpp -o checkers/checker.bc
#llvm-link-3.9 $bitcode checkers/checker.bc -o $protected
opt-3.9 -load build/lib/libcontrolflow-integrity.so $bitcode -cf-integrity -o $protected
#clang++-3.9 -lcurses -lunwind -lunwind-x86_64 $protected -o out
#llc-3.9 -filetype=obj out.bc
#g++ out.o -o out -lunwind-x86_64 -lunwind #-L/usr/lib/x86_64-linux-gnu -Wl,-rpath=/usr/lib/x86_64-linux-gnu -lunwind-x86_64 -lunwind  out.o -o out

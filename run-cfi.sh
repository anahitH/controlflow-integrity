#!/bin/bash

# $1 source file 
# $2 rtlib bitcode

CFI_PATH=/home/anahitik/TUM_S17/SIP/State_inspection/controlflow-integrity
CFI_BUILD=$CFI_PATH/build/lib/

llvm-link $1 $CFI_PATH/checkers/checker.bc -o out.bc
opt -load $CFI_BUILD/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
clang++ -std=c++0x -stdlib=libc++ -lunwind -lunwind-x86_64 -rdynamic out.bc -o protected

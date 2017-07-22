#!/bin/bash

# $1 source file 
# $2 rtlib bitcode

CFI_PATH=/home/anahitik/TUM_S17/SIP/State_inspection/controlflow-integrity
CFI_BUILD=$CFI_PATH/build/lib/
bitcode=$1

llvm-link-3.9 $1 $CFI_RTILB_PATH/checker.bc -o out.bc

opt-3.9 -load $CFI_LIB/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
clang++-3.9 -lunwind -lunwind-x86_64 -rdynamic -std=c++0x -stdlib=libc++ out.bc -o out

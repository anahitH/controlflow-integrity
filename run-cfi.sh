#!/bin/bash

CFI_LIB=./build/lib
CFI_RTILB_PATH=.//checkers

bitcode=$1

llvm-link $1 $CFI_RTILB_PATH/checker.bc -o out.bc

opt -load $CFI_LIB/libcontrolflow-integrity.so out.bc -cf-integrity -o out.bc
clang++ -lunwind -lunwind-x86_64 -rdynamic -std=c++0x -stdlib=libc++ out.bc -o out


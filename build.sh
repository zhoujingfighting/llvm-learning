#!/usr/bin/env bash
clang++ -g -O3  \
      Lexer.cpp \
      Parser.cpp \
      toy.cpp \
      Codegen.cpp \
      `llvm-config --cxxflags --ldflags --system-libs --libs core` -o toy
#!/bin/bash
clear

# shaderc -f vs_simple.vert -o vs_simple.hpp --type vertex --platform linux --platform 120 --bin2c
# shaderc -f fs_simple.frag -o fs_simple.hpp --type fragment --platform linux --platform 120 --bin2c

bear -- ccache clang++ \
    -march=native \
    -flto=jobserver \
    -std=gnu++26 \
    -ffunction-sections -fdata-sections \
    -fPIC \
    -fopenmp-simd \
    -fno-ident \
    -fno-short-enums \
    -Wall \
    -Wextra \
    -Wno-gcc-compat -Wno-incompatible-pointer-types-discards-qualifiers \
    main.cpp \
    -Og -ggdb3 \
    -fno-rtti -fno-exceptions -fno-threadsafe-statics \
    -fno-unwind-tables \
    -lSDL3 \
    -lmimalloc \
    -lelf -lunwind
# -lbgfx -lbimg -lbx -lassimp -ldl -lpthread

#!/bin/bash
shaderc -f vs_simple.sc -o shaders/vs_simple.bin --type vertex --platform linux -p glsl
shaderc -f fs_simple.sc -o shaders/fs_simple.bin --type fragment --platform linux -p glsl

clang++ -std=gnu++2b main.cpp $(sdl3-config --cflags) \
    -lbgfx -lbimg -lbx -lassimp $(sdl3-config --libs) -ldl -lpthread -o fbxtoy

#!/bin/bash
clear

vertex_filename='vs_simple'
fragment_filename='fs_simple'

glsl_version=120

vertex_filepath="$vertex_filename"'.vert'
fragment_filepath="$fragment_filename"'.frag'
vertex_compiled_filepath="$vertex_filename"'.bin'
fragment_compiled_filepath="$fragment_filename"'.bin'

compile_shader() {
    input="$1"
    output="$2"
    type="$3"

    if [ ! -f "$output" ] || [[ "$input" -nt "$output" ]]; then
        shaderc -f "$input" -o "$output" --type "$type" --platform linux --profile "$glsl_version"

        echo 'Making '"$output"
    fi
}

compile_shader "$vertex_filepath" "$vertex_compiled_filepath" 'vertex'
compile_shader "$fragment_filepath" "$fragment_compiled_filepath" 'fragment'

echo 'Making executable'

bear -- ccache clang++ \
    -march=native \
    -flto=jobserver \
    -fuse-ld=mold \
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
    -D DEBUG \
    -Og -ggdb3 \
    -fno-rtti -fno-exceptions -fno-threadsafe-statics \
    -fno-unwind-tables \
    -lSDL3 \
    -lmimalloc \
    -lelf -lunwind
# -lbgfx -lbimg -lbx -lassimp -ldl -lpthread

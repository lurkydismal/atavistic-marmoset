#!/bin/bash
clear

set -e

common_flags='-march=native -std=gnu++26 -ffunction-sections -fdata-sections -fPIC -fopenmp-simd -fno-short-enums -Wall -Wextra -Wno-gcc-compat -Wno-incompatible-pointer-types-discards-qualifiers -ggdb3 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables'
compiler_flags='-flto=jobserver -fno-ident -D DEBUG -D BX_CONFIG_DEBUG=1 -Og'
linker_flags=" -flto -fuse-ld=mold -Wl,-O1 -Wl,--gc-sections -Wl,--no-eh-frame-hdr -rdynamic -Wl,-rpath,\$ORIGIN"
glsl_version=120

vertex_filename='vs'
fragment_filename='fs'

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

source_files=(
    'FPS.cpp'
    'main.cpp'
    'runtime.cpp'
    'shader.cpp'
    'vsync.cpp'
)

for source_file in "${source_files[@]}"; do
    echo 'Making '"$source_file"

    bear -- ccache clang++ $common_flags $compiler_flags -c "$source_file"
done

echo 'Making executable'

clang++ $common_flags $linker_flags ${source_files[@]/%.cpp/.o} -lSDL3 -lbgfx -lassimp -lmimalloc -lelf -lunwind

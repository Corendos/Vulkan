#!/usr/bin/zsh

if [ "$1" = "release" ]; then
    echo "Building Release"
    cd build/
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j4
elif [ "$1" = "debug" ]; then
    echo "Building Release"
    cd build/
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    make -j4
else
    echo "Positional parameter not specified (release/debug)"
fi
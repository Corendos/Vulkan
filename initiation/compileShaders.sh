#!/usr/bin/zsh

temp="$(dirname "$0")"

$VULKAN_SDK/bin/glslangValidator -V $temp/resources/shaders/shader.vert -o $temp/resources/shaders/build/vert.spv
$VULKAN_SDK/bin/glslangValidator -V $temp/resources/shaders/shader.frag -o $temp/resources/shaders/build/frag.spv
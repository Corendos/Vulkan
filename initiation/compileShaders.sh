#!/usr/bin/zsh

temp="$(dirname "$0")"

$VULKAN_SDK/bin/glslangValidator -V $temp/shaders/shader.vert -o $temp/shaders/build/vert.spv
$VULKAN_SDK/bin/glslangValidator -V $temp/shaders/shader.frag -o $temp/shaders/build/frag.spv
$VULKAN_SDK/bin/glslangValidator -V $temp/shaders/shaderColor.frag -o $temp/shaders/build/fragColor.spv
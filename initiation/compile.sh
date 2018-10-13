#!/usr/bin/zsh

$VULKAN_SDK/bin/glslangValidator -V shaders/shader.vert -o shaders/build/vert.spv
$VULKAN_SDK/bin/glslangValidator -V shaders/shader.frag -o shaders/build/frag.spv
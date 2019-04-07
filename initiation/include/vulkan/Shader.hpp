#ifndef SHADER
#define SHADER

#include <string>

#include <vulkan/vulkan.hpp>

#include "vulkan/VulkanContext.hpp"

class Shader {
    public:
        using Pair = std::pair<vk::ShaderModule, vk::PipelineShaderStageCreateInfo>;
        static Pair create(VulkanContext& context,
                           std::string filename,
                           vk::ShaderStageFlagBits shaderStage,
                           std::string entryPointName);
};

#endif
#include <vector>

#include "vulkan/Shader.hpp"

#include "files/File.hpp"
Shader::Pair Shader::create(VulkanContext& context,
                            std::string filename, 
                            vk::ShaderStageFlagBits shaderStage,
                            std::string entryPointName) {
    std::vector<char> code = File::readFile(filename);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(code.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    vk::ShaderModule module = context.getDevice().createShaderModule(createInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.setStage(shaderStage);
    shaderStageInfo.setModule(module);
    shaderStageInfo.setPName(entryPointName.c_str());

    return std::make_pair(module, shaderStageInfo);
}
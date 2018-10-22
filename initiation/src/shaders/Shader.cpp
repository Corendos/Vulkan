#include <vector>
#include <iostream>

#include "shaders/Shader.hpp"

#include "files/File.hpp"

Shader::Shader() {}

Shader::Shader(std::string filename, VkShaderStageFlagBits shaderStage, std::string entryPointName) :
    mStage(shaderStage), mEntryPointName(entryPointName), mFilename(filename) {
    mCode = File::readFile(mFilename);

    mModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    mModuleInfo.pCode = reinterpret_cast<const uint32_t*>(mCode.data());
    mModuleInfo.codeSize = mCode.size();
}

Shader::Shader(Shader&& other) {
    *this = std::move(other);
}

Shader& Shader::operator=(Shader&& other) {
    mEntryPointName = other.mEntryPointName;
    mFilename = other.mFilename;
    mModule = other.mModule;
    mModuleInfo = other.mModuleInfo;
    mStage = other.mStage;
    mCode = std::move(other.mCode);
    mCreated = other.mCreated;

    other.mCreated = false;

    return *this;
}

void Shader::create(VkDevice device) {
    if (mCreated) {
        return;
    }

    if (vkCreateShaderModule(device, &mModuleInfo, nullptr, &mModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    mCreated = true;
}

void Shader::destroy(VkDevice device) {
    if (mCreated) {
        vkDestroyShaderModule(device, mModule, nullptr);
        mCreated = false;
    }
}

VkPipelineShaderStageCreateInfo Shader::getCreateInfo() const {
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = mStage;
    shaderStageInfo.module = mModule;
    shaderStageInfo.pName = mEntryPointName.c_str();

    return shaderStageInfo;
}

Shader::~Shader() {}
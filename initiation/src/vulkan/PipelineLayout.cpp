#include "vulkan/PipelineLayout.hpp"

#include "camera/CameraInfo.hpp"

PipelineLayout::PipelineLayout() {
    mInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
}

void PipelineLayout::create(VkDevice device) {
    if (mCreated) {
        return;
    }

    mInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorSetLayouts.size());
    mInfo.pSetLayouts = mDescriptorSetLayouts.data();
    mInfo.pushConstantRangeCount = static_cast<uint32_t>(mPushConstantRanges.size());
    mInfo.pPushConstantRanges = mPushConstantRanges.data();

    if (vkCreatePipelineLayout(device, &mInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    mCreated = true;
}

void PipelineLayout::destroy(VkDevice device) {
    if (mCreated) {
        vkDestroyPipelineLayout(device, mHandler, nullptr);
        mDescriptorSetLayouts.clear();
        mCreated = false;
    }
}

void PipelineLayout::setFlags(VkPipelineLayoutCreateFlags flags) {
    mInfo.flags = flags;
}

void PipelineLayout::setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> layouts) {
    mDescriptorSetLayouts = layouts;
}

void PipelineLayout::setPushConstants(std::vector<VkPushConstantRange> pushConstants) {
    mPushConstantRanges = pushConstants;
}


VkPipelineLayout PipelineLayout::getHandler() const {
    return mHandler;
}
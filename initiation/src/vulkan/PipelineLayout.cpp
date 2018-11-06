#include "vulkan/PipelineLayout.hpp"

#include "camera/CameraInfo.hpp"

PipelineLayout::PipelineLayout() {
    mInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
}

void PipelineLayout::create(VkDevice device) {
    if (mCreated) {
        return;
    }

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

void PipelineLayout::addDescriptorSetLayout(VkDescriptorSetLayout layout) {
    mDescriptorSetLayouts.push_back(layout);

    mInfo.setLayoutCount = mDescriptorSetLayouts.size();
    mInfo.pSetLayouts = mDescriptorSetLayouts.data();
}

VkPipelineLayout PipelineLayout::getHandler() const {
    return mHandler;
}
#include "vulkan/SubpassDependency.hpp"

VkSubpassDependency SubpassDependency::getDependency() const {
    return mDependency;
}

void SubpassDependency::setSourceSubpass(uint32_t index) {
    mDependency.srcSubpass = index;
}

void SubpassDependency::setDestinationSubpass(uint32_t index) {
    mDependency.dstSubpass = index;
}

void SubpassDependency::setSourceStageMask(VkPipelineStageFlags flags) {
    mDependency.srcStageMask = flags;
}

void SubpassDependency::setDestinationStageMask(VkPipelineStageFlags flags) {
    mDependency.dstStageMask = flags;
}

void SubpassDependency::setSourceAccessMask(VkAccessFlags flags) {
    mDependency.srcAccessMask = flags;
}

void SubpassDependency::setDestinationAccessMask(VkAccessFlags flags) {
    mDependency.dstAccessMask = flags;
}

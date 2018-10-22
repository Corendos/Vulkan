#ifndef SUBPASSDEPENDENCY
#define SUBPASSDEPENDENCY

#include <vulkan/vulkan.h>

class SubpassDependency {
    public:
        VkSubpassDependency getDependency() const;

        void setSourceSubpass(uint32_t index);
        void setDestinationSubpass(uint32_t index);
        void setSourceStageMask(VkPipelineStageFlags flags);
        void setDestinationStageMask(VkPipelineStageFlags flags);
        void setSourceAccessMask(VkAccessFlags flags);
        void setDestinationAccessMask(VkAccessFlags flags);

    private:
        VkSubpassDependency mDependency{};
};

#endif
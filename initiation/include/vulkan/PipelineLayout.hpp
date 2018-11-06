#ifndef PIPELINELAYOUT
#define PIPELINELAYOUT

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

class PipelineLayout {
    public:
        PipelineLayout();

        void create(VkDevice device);
        void destroy(VkDevice device);
        VkPipelineLayout getHandler() const;

        void setFlags(VkPipelineLayoutCreateFlags flags);
        void setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> layouts);
        void setPushConstants(std::vector<VkPushConstantRange> pushConstants);

    private:
        VkPipelineLayout mHandler;
        VkPipelineLayoutCreateInfo mInfo{};

        std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
        std::vector<VkPushConstantRange> mPushConstantRanges;

        bool mCreated{false};
};

#endif
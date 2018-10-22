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

        void addDescriptorSetLayout(VkDescriptorSetLayout layout);
        VkPipelineLayout getHandler() const;

    private:
        VkPipelineLayout mHandler;
        VkPipelineLayoutCreateInfo mInfo{};

        std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;

        bool mCreated{false};
};

#endif
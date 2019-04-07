#ifndef GRAPHICSPIPELINE
#define GRAPHICSPIPELINE

#include <vector>
#include <algorithm>

#include <vulkan/vulkan.hpp>

#include "vulkan/VulkanContext.hpp"

class GraphicsPipeline {
    public:
        static vk::Pipeline create(VulkanContext& context,
                                   vk::RenderPass renderPass,
                                   vk::Extent2D extent,
                                   vk::PipelineLayout pipelineLayout,
                                   vk::PipelineShaderStageCreateInfo vertexShader,
                                   vk::PipelineShaderStageCreateInfo fragmentShader);
};

#endif
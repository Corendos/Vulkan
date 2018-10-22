#ifndef GRAPHICSPIPELINE
#define GRAPHICSPIPELINE

#include <vector>

#include <vulkan/vulkan.h>
#include "device/PipelineLayout.hpp"
#include "device/RenderPass.hpp"
#include "shaders/Shader.hpp"
#include "Vertex.hpp"

class GraphicsPipeline {
    public:
        GraphicsPipeline();
        void create(VkDevice device);
        void destroy(VkDevice device);

        void addShader(VkPipelineShaderStageCreateInfo info);
        void setRenderPass(RenderPass& renderPass);
        void setExtent(VkExtent2D extent);

        VkPipeline getHandler() const;
        PipelineLayout& getLayout();

    private:
        VkPipeline mHandler;
        VkGraphicsPipelineCreateInfo mInfo{};
        std::vector<VkPipelineShaderStageCreateInfo> mShaderInfos;
        RenderPass mRenderPass;

        VkExtent2D mExtent;

        PipelineLayout mLayout;

        bool mCreated{false};
        bool mLayoutCreated{false};
};

#endif
#ifndef GRAPHICSPIPELINE
#define GRAPHICSPIPELINE

#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>
#include "vulkan/PipelineLayout.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Shader.hpp"

class GraphicsPipeline {
    public:
        GraphicsPipeline();
        void create(VkDevice device);
        void destroy(VkDevice device);

        void addShader(Shader& shader);
        void setRenderPass(RenderPass& renderPass);
        void setExtent(VkExtent2D extent);

        VkPipeline getHandler() const;
        PipelineLayout& getLayout();

    private:
        VkPipeline mHandler;
        VkGraphicsPipelineCreateInfo mInfo{};
        std::vector<Shader*> mShaders;
        RenderPass* mRenderPass;

        VkExtent2D mExtent;

        PipelineLayout mLayout;

        bool mCreated{false};
};

#endif
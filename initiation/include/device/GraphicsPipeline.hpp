#ifndef GRAPHICSPIPELINE
#define GRAPHICSPIPELINE

#include <vector>
#include <algorithm>

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

        void addShader(Shader& shader);
        void setRenderPass(RenderPass& renderPass);
        void setExtent(VkExtent2D extent);

        VkPipeline getHandler() const;
        PipelineLayout& getLayout();

    private:
        VkPipeline mHandler;
        VkGraphicsPipelineCreateInfo mInfo{};
        std::vector<Shader*> mShaders;
        RenderPass mRenderPass;

        VkExtent2D mExtent;

        PipelineLayout mLayout;

        bool mCreated{false};
        bool mLayoutCreated{false};
};

#endif
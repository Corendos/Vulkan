#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/Vertex.hpp"

GraphicsPipeline::GraphicsPipeline() {
    mInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
}

void GraphicsPipeline::create(VkDevice device) {
    if (mCreated) {
        return;
    }

    if (mRenderPass == vk::RenderPass()) {
        throw std::runtime_error("Missing mandatory pointer");
    }

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewPort{};
    viewPort.x = 0;
    viewPort.y = 0;
    viewPort.width = (float)mExtent.width;
    viewPort.height = (float)mExtent.height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    VkRect2D scissors{};
    scissors.offset = {0, 0};
    scissors.extent = mExtent;
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewPort;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissors;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendAttachmentState normalBlendAttachment{};
    normalBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    normalBlendAttachment.blendEnable = VK_FALSE;
    normalBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    normalBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    normalBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    normalBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    normalBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    normalBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendAttachmentState blendStates[] = {colorBlendAttachment, normalBlendAttachment};

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 2;
    colorBlending.pAttachments = blendStates;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineDepthStencilStateCreateInfo stencilInfo{};
    stencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stencilInfo.depthTestEnable = VK_TRUE;
    stencilInfo.depthWriteEnable = VK_TRUE;
    stencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    stencilInfo.depthBoundsTestEnable = VK_FALSE;
    stencilInfo.minDepthBounds = 0.0f;
    stencilInfo.maxDepthBounds = 1.0f;
    stencilInfo.stencilTestEnable = VK_FALSE;
    stencilInfo.front = {};
    stencilInfo.back = {};

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(mShaders.size());
    std::transform(mShaders.begin(), mShaders.end(),
                   shaderStages.begin(), [](const Shader* s) { return s->getCreateInfo(); });

    mInfo.stageCount = static_cast<uint32_t>(mShaders.size());
    mInfo.pStages = shaderStages.data();
    mInfo.pVertexInputState = &vertexInputInfo;
    mInfo.pInputAssemblyState = &inputAssembly;
    mInfo.pViewportState = &viewportState;
    mInfo.pRasterizationState = &rasterizer;
    mInfo.pMultisampleState = &multisampling;
    mInfo.pDepthStencilState = &stencilInfo;
    mInfo.pColorBlendState = &colorBlending;
    mInfo.pDynamicState = nullptr;
    mInfo.layout = mLayout.getHandler();
    mInfo.renderPass = mRenderPass;
    mInfo.subpass = 0;
    mInfo.basePipelineHandle = VK_NULL_HANDLE;
    mInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &mInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    mCreated = true;
}

void GraphicsPipeline::destroy(VkDevice device) {
    if (mCreated) {
        mLayout.destroy(device);
        vkDestroyPipeline(device, mHandler, nullptr);
        mShaders.clear();
        mCreated = false;
    }
}

void GraphicsPipeline::addShader(Shader& shader) {
    mShaders.push_back(&shader);
}

void GraphicsPipeline::setRenderPass(vk::RenderPass& renderPass) {
    mRenderPass = renderPass;
}

void GraphicsPipeline::setExtent(VkExtent2D extent) {
    mExtent = extent;
}

void GraphicsPipeline::setPipelineLayout(PipelineLayout& layout) {
    mLayout = layout;
}

VkPipeline GraphicsPipeline::getHandler() const {
    return mHandler;
}

PipelineLayout& GraphicsPipeline::getLayout() {
    return mLayout;
}


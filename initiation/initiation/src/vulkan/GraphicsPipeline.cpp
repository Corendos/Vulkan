#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/Vertex.hpp"

vk::Pipeline GraphicsPipeline::create(VulkanContext& context,
                                      vk::RenderPass renderPass,
                                      vk::Extent2D extent,
                                      vk::PipelineLayout pipelineLayout,
                                      vk::PipelineShaderStageCreateInfo vertexShader,
                                      vk::PipelineShaderStageCreateInfo fragmentShader) {
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptionCount(1);
    vertexInputInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputInfo.setVertexAttributeDescriptionCount(attributeDescriptions.size());
    vertexInputInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
    inputAssembly.setPrimitiveRestartEnable(VK_FALSE);

    vk::Viewport viewPort;
    viewPort.setX(0);
    viewPort.setY(0);
    viewPort.setWidth(extent.width);
    viewPort.setHeight(extent.height);
    viewPort.setMinDepth(0.0f);
    viewPort.setMaxDepth(1.0f);

    vk::Rect2D scissors{{0, 0}, extent};
    
    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setViewportCount(1);
    viewportState.setPViewports(&viewPort);
    viewportState.setScissorCount(1);
    viewportState.setPScissors(&scissors);

    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.setDepthClampEnable(VK_FALSE);
    rasterizer.setRasterizerDiscardEnable(VK_FALSE);
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
    rasterizer.setDepthBiasEnable(VK_FALSE);
    rasterizer.setDepthBiasConstantFactor(0.0f);
    rasterizer.setDepthBiasClamp(0.0f);
    rasterizer.setDepthBiasSlopeFactor(0.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.setSampleShadingEnable(VK_FALSE);
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    multisampling.setMinSampleShading(1.0f);
    multisampling.setAlphaToCoverageEnable(VK_FALSE);
    multisampling.setAlphaToOneEnable(VK_FALSE);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    colorBlendAttachment.setBlendEnable(VK_FALSE);
    colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
    colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
    colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
    colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendAttachmentState normalBlendAttachment;
    normalBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    normalBlendAttachment.setBlendEnable(VK_FALSE);
    normalBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
    normalBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
    normalBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
    normalBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    normalBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    normalBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendAttachmentState blendStates[] = {colorBlendAttachment, normalBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.setLogicOpEnable(VK_FALSE);
    colorBlending.setLogicOp(vk::LogicOp::eCopy);
    colorBlending.setAttachmentCount(2);
    colorBlending.setPAttachments(blendStates);
    colorBlending.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

    vk::DynamicState dynamicStates[] = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eLineWidth
    };

    vk::PipelineDepthStencilStateCreateInfo stencilInfo;
    stencilInfo.setDepthTestEnable(VK_TRUE);
    stencilInfo.setDepthWriteEnable(VK_TRUE);
    stencilInfo.setDepthCompareOp(vk::CompareOp::eLess);
    stencilInfo.setDepthBoundsTestEnable(VK_FALSE);
    stencilInfo.setMinDepthBounds(0.0f);
    stencilInfo.setMaxDepthBounds(1.0f);
    stencilInfo.setStencilTestEnable(VK_FALSE);

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vertexShader, fragmentShader};

    vk::GraphicsPipelineCreateInfo createInfo;
    createInfo.setStageCount(2);
    createInfo.setPStages(shaderStages.data());
    createInfo.setPVertexInputState(&vertexInputInfo);
    createInfo.setPInputAssemblyState(&inputAssembly);
    createInfo.setPViewportState(&viewportState);
    createInfo.setPRasterizationState(&rasterizer);
    createInfo.setPMultisampleState(&multisampling);
    createInfo.setPDepthStencilState(&stencilInfo);
    createInfo.setPColorBlendState(&colorBlending);
    createInfo.setLayout(pipelineLayout);
    createInfo.setRenderPass(renderPass);
    createInfo.setSubpass(0);

    return context.getDevice().createGraphicsPipeline(vk::PipelineCache(), createInfo);
}

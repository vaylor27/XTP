
#include "SimpleShaderObject.h"
#include "XTPVulkan.h"

void SimpleShaderObject::cleanUp() {
    vkDestroyPipeline(XTPVulkan::device, pipeline, nullptr);
    vkDestroyPipelineLayout(XTPVulkan::device, layout, nullptr);
}

bool SimpleShaderObject::canObjectsTick() {
    return false;
}

void SimpleShaderObject::prepareForRender(const VkCommandBuffer& commandBuffer) {
    bindShader(commandBuffer);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(XTPVulkan::swapchainExtent.width);
    viewport.height = static_cast<float>(XTPVulkan::swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = XTPVulkan::swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void SimpleShaderObject::bindShader(const VkCommandBuffer& commandBuffer) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void SimpleShaderObject::createGraphicsPipeline() {
    VkShaderModule vertShaderModule = createShaderModule(FileUtil::readFile(this->vertexShaderPath));
    VkShaderModule fragShaderModule = createShaderModule(FileUtil::readFile(this->fragmentShaderPath));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    std::vector<VkVertexInputBindingDescription> bindingDescriptions = getVertexInput().getBindingDescriptions();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = getVertexInput().getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    const std::vector<VkDynamicState> &dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    if (!VulkanRenderInfo::INSTANCE->getPhysicalDeviceFeatures().depthClamp && properties.depthClamp) {
        XTPVulkan::logger->logCritical("depthClamp Must Be Enabled In getPhysicalDeviceFeatures() To Use Depth Clamping!");
    }
    if (!VulkanRenderInfo::INSTANCE->getPhysicalDeviceFeatures().fillModeNonSolid && !(properties.polygonMode == VK_POLYGON_MODE_FILL || properties.polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV)) {
        XTPVulkan::logger->logCritical("If fillModeNonSolid Is Disabled In getPhysicalDeviceFeatures(), The Polygon Mode MUST Be Either VK_POLYGON_MODE_FILL or VK_POLYGON_MODE_FILL_RECTANGLE_NV!");
    }
    if (properties.lineWidth > 1.0f && !VulkanRenderInfo::INSTANCE->getPhysicalDeviceFeatures().wideLines) {
        XTPVulkan::logger->logCritical("To Have Lines Wider Than 1.0f, wideLines Must Be Enabled In getPhysicalDeviceFeatures()!");
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = properties.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = properties.depthClamp;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = properties.polygonMode;
    rasterizer.lineWidth = properties.lineWidth;
    rasterizer.cullMode = properties.cullMode;
    rasterizer.frontFace = properties.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;

    //TODO: pass as param
    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    //TODO: allow uniforms
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(XTPVulkan::device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        XTPVulkan::logger->logCritical("Failed To Create Pipeline Layout!");
    }

    this->layout = pipelineLayout;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &properties.colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = properties.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(XTPVulkan::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        XTPVulkan::logger->logCritical("Failed To Create Graphics Pipeline!");
    }

    this->pipeline = graphicsPipeline;

    vkDestroyShaderModule(XTPVulkan::device, fragShaderModule, nullptr);
    vkDestroyShaderModule(XTPVulkan::device, vertShaderModule, nullptr);

}

VkShaderModule SimpleShaderObject::createShaderModule(const std::vector<char> &shaderBytecode) {
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderBytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytecode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(XTPVulkan::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        XTPVulkan::logger->logCritical("Failed To Create Shader Module!");
    }

    return shaderModule;
}


#ifndef DEFAULTSHADEROBJECT_H
#define DEFAULTSHADEROBJECT_H

#define SHADER_INPUT_FLOAT VK_FORMAT_R32_SFLOAT
#define SHADER_INPUT_VECTOR2F VK_FORMAT_R32G32_SFLOAT
#define SHADER_INPUT_VECTOR3F VK_FORMAT_R32G32B32_SFLOAT
#define SHADER_INPUT_VECTOR4F VK_FORMAT_R32G32B32A32_SFLOAT

#include <any>

#include "FileUtil.h"

#include "ShaderObject.h"
#include "XTPVulkan.h"
#include "vulkan/vulkan.h"

struct VertexAttribute {
    //The location, as specified in the shader
    uint32_t location;
    //The format
    VkFormat format;
    //If the VertexInput data array member contains multiple data types (e.g a struct of position and color), how far this attribute is offset from the beginning of the data menber type
    //It is possible to use the offsetof macro to calculate this
    uint32_t offset;
};

struct VertexInputData {
    //A list of attributes describing how the data is to be passed to the vertex shader
    std::vector<VertexAttribute> attributes;
    //The distance between two VertexDataObjects. Can be obtained with sizeof()
    uint32_t stride;
    //if false, this is assumed to be per instance data, else it is assumed to be per vertex data.
    bool perVertex = true;
};

struct VertexInput {
    std::vector<VertexInputData> vertexData;

    [[nodiscard]] std::vector<VkVertexInputBindingDescription> getBindingDescriptions() const {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;

        for (int i = 0; i < vertexData.size(); ++i) {
            VkVertexInputBindingDescription bindingDescription {};
            bindingDescription.binding = i;
            bindingDescription.stride = vertexData[i].stride;
            bindingDescription.inputRate = vertexData[i].perVertex ? VK_VERTEX_INPUT_RATE_VERTEX: VK_VERTEX_INPUT_RATE_INSTANCE;

            bindingDescriptions.emplace_back(bindingDescription);
        }
        return bindingDescriptions;
    }

    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        for (int i = 0; i < vertexData.size(); ++i) {
            for (VertexAttribute attribute : vertexData[i].attributes) {
                VkVertexInputAttributeDescription attributeDescription {};
                attributeDescription.binding = i;
                attributeDescription.location = attribute.location;
                attributeDescription.format = attribute.format;
                attributeDescription.offset = attribute.offset;

                attributeDescriptions.emplace_back(attributeDescription);
            }
        }

        return attributeDescriptions;
    }
};

struct Descriptor {
    int set;
    int binding;
    VkShaderStageFlags stage;
    VkSampler* immutableSamplers;
    uint32_t descriptorCount;
    VkDescriptorType type;
};

struct ShaderProperties: ShaderData{
    VkRenderPass renderPass = {
        XTPVulkan::renderPass
    };
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 depthClamp = VK_FALSE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    float lineWidth = 1.0;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };
};

struct PushConstantInfo {
    VkShaderStageFlags pushConstantShaderStages;
    uint32_t size;
    uint32_t offset;
};

class SimpleShaderObject: public ShaderObject {
public:
    VkPipelineLayout pipelineLayout {};
    VkPipeline pipeline {};
    ShaderProperties properties;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts {};
    bool deleted = false;
    uint32_t sets;
    std::unordered_map<uint32_t, uint32_t> bindings;
    std::unordered_map<int, std::unordered_map<int, Descriptor>> descriptors {};
    std::vector<PushConstantInfo> pushConstants;

    // ReSharper disable once CppPossiblyUninitializedMember
    SimpleShaderObject(const std::string &vertexShaderPath,
                 const std::string &fragmentShaderPath,
                 const ShaderProperties &properties,
                 const std::vector<PushConstantInfo> &pushConstants
                 ): ShaderObject(vertexShaderPath, fragmentShaderPath), properties(properties), pushConstants(pushConstants) {}

    virtual void initShader() = 0;

    virtual void createBuffers() = 0;

    VkPipelineLayout getLayout() override {
        return pipelineLayout;
    }

    // ReSharper disable once CppNotAllPathsReturnValue
    virtual XTPVulkan::DescriptorSet createDescriptorSet(const uint32_t set) {
        uint32_t descriptorCount = 0;
        for (const auto [binding, descriptor] : descriptors[static_cast<int>(set)]) {
            descriptorCount += descriptor.descriptorCount;
        }
        XTPVulkan::allocatorPool->SetPoolSizeMultiplier(descriptors[static_cast<int>(set)][0].type, static_cast<float>(descriptorCount));

        auto handle = XTPVulkan::allocatorPool->GetAllocator();

        VkDescriptorSet newSet;
        if (handle.Allocate(descriptorSetLayouts[set], newSet)) {
            return XTPVulkan::DescriptorSet {newSet, descriptors[static_cast<int>(set)][0].type};
        }
        XTPVulkan::logger->logCritical("Failed To Allocate Descriptor Set!");
    }

    void init() override {
        for (auto variable : getDescriptors()) {
            descriptors[variable.set][variable.binding] = variable;
        }
        for (int i = 0; i < descriptors.size(); ++i) {
            if (descriptors[i].empty()) {
                XTPVulkan::logger->logCritical("Descriptor Set Must Contain At Least One Binding!");
            }
        }
        createGraphicsPipeline();
        initShader();
        createBuffers();
    }

    bool hasDeleted() override {
        return deleted;
    }

    virtual VertexInput getVertexInput() = 0;

    virtual std::vector<Descriptor> getDescriptors() = 0;

    void cleanUp() override {
        vkDestroyPipeline(XTPVulkan::device, pipeline, nullptr);
        vkDestroyPipelineLayout(XTPVulkan::device, pipelineLayout, nullptr);
        for (const VkDescriptorSetLayout& descriptorSetLayout : getDescriptorSetLayouts()) {
            vkDestroyDescriptorSetLayout(XTPVulkan::device, descriptorSetLayout, nullptr);
        }

        deleted = true;
    }

    bool canObjectsTick() override{
        return false;
    }

    void prepareForRender(const VkCommandBuffer& commandBuffer) override{
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

    void bindShader(const VkCommandBuffer& commandBuffer) const {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void createGraphicsPipeline() {
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
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;


        std::vector<VkPushConstantRange> pushConstantRanges(pushConstants.size());
        for (int i = 0; i < pushConstants.size(); ++i) {
            PushConstantInfo* pushConstant = &pushConstants[i];

            VkPushConstantRange pushConstantRange;
            pushConstantRange.offset = pushConstant->offset;
            pushConstantRange.size = pushConstant->size;
            pushConstantRange.stageFlags = pushConstant->pushConstantShaderStages;

            pushConstantRanges[i] = pushConstantRange;
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = getDescriptorSetLayouts();
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

        if (vkCreatePipelineLayout(XTPVulkan::device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            XTPVulkan::logger->logCritical("Failed To Create Pipeline Layout!");
        }


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &properties.depthStencilState;
        pipelineInfo.pColorBlendState = &properties.colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = properties.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(XTPVulkan::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            XTPVulkan::logger->logCritical("Failed To Create Graphics Pipeline!");
        }

        vkDestroyShaderModule(XTPVulkan::device, fragShaderModule, nullptr);
        vkDestroyShaderModule(XTPVulkan::device, vertShaderModule, nullptr);

    }

    std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() {
        if (descriptorSetLayouts.empty()) {

            for (const auto& [set, bindings] : descriptors) {
                std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
                if (set + 1 > sets) {
                    sets = set + 1;
                }
                for (auto [binding, uniformVariable] : bindings) {
                    if (binding + 1 > binding) {
                        this->bindings[set] = binding + 1;
                    }
                    VkDescriptorSetLayoutBinding layoutBinding {};
                    layoutBinding.binding = binding;
                    layoutBinding.descriptorType = uniformVariable.type;
                    layoutBinding.descriptorCount = uniformVariable.descriptorCount; //Generally should be 1 or the array size of the buffer array
                    layoutBinding.stageFlags = uniformVariable.stage;
                    layoutBinding.pImmutableSamplers = uniformVariable.immutableSamplers;
                    layoutBindings.emplace_back(layoutBinding);
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = layoutBindings.size();
                layoutInfo.pBindings = layoutBindings.data();

                VkDescriptorSetLayout layout;
                if (vkCreateDescriptorSetLayout(XTPVulkan::device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
                descriptorSetLayouts.emplace_back(layout);
            }
        }

        return descriptorSetLayouts;
    }

    static VkShaderModule createShaderModule(const std::vector<char> &shaderBytecode) {
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

    template <class T> static void bindPushConstant(VkCommandBuffer commandBuffer, const T &pushConstant, SimpleShaderObject* object, int pushConstantIndex = 0) {
        vkCmdPushConstants(commandBuffer, object->pipelineLayout, object->pushConstants[pushConstantIndex], object->pushConstants[pushConstantIndex].offset, object->pushConstants[pushConstantIndex].size, &pushConstant);
    }
};



#endif //DEFAULTSHADEROBJECT_H

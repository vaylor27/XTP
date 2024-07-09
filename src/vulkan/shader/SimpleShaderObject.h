
#ifndef DEFAULTSHADEROBJECT_H
#define DEFAULTSHADEROBJECT_H

#define SHADER_INPUT_FLOAT VK_FORMAT_R32_SFLOAT
#define SHADER_INPUT_VECTOR2F VK_FORMAT_R32G32_SFLOAT
#define SHADER_INPUT_VECTOR3F VK_FORMAT_R32G32B32_SFLOAT
#define SHADER_INPUT_VECTOR4F VK_FORMAT_R32G32B32A32_SFLOAT

#include <iostream>
#include <utility>

#include "FileUtil.h"
#include <vulkan/vulkan.h>

#include "ShaderObject.h"

struct VertexAttribute {
    //The location, as specified in the shader
    uint32_t location;
    //The format
    VkFormat format;
    //If the VertexInput data array member contains multiple data types (e.g a struct of position and color), how far this attribute is offset from the beginning of the data menber type
    //It is possible to use the offsetof macro to calculate this
    uint32_t offset;
};

struct VertexData {
    //A list of attributes describing how the data is to be passed to the vertex shader
    std::vector<VertexAttribute> attributes;
    //The distance between two VertexDataObjects. Can be obtained with sizeof()
    uint32_t stride;
    //if false, this is assumed to be per instance data, else it is assumed to be per vertex data.
    bool perVertex = true;
};

struct VertexInput {
    std::vector<VertexData> vertexData;

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

struct UniformVariable {
    //TODO
};

struct ShaderProperties: ShaderData{
    VkRenderPass renderPass; //MUST be set manually
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
    std::vector<UniformVariable> uniformVariables {};
};

class SimpleShaderObject: public ShaderObject {
public:
    VkPipelineLayout layout;
    VkPipeline pipeline;
    ShaderProperties properties;

    // ReSharper disable once CppPossiblyUninitializedMember
    SimpleShaderObject(const std::string &vertexShaderPath,
                 const std::string &fragmentShaderPath,
                 ShaderProperties properties): ShaderObject(vertexShaderPath, fragmentShaderPath), properties(std::move(properties)) {}

    void init() override {
        createGraphicsPipeline();
    }

    virtual VertexInput getVertexInput() = 0;

    void cleanUp() override;

    bool canObjectsTick() override;

    void prepareForRender(const VkCommandBuffer &commandBuffer) override;

    void bindShader(const VkCommandBuffer &commandBuffer) const;

    void createGraphicsPipeline();

    static VkShaderModule createShaderModule(const std::vector<char> &shaderBytecode);
};



#endif //DEFAULTSHADEROBJECT_H

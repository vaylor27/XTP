
#ifndef TESTSHADEROBJECT_H
#define TESTSHADEROBJECT_H
#include <glm/glm.hpp>

#include "renderable/SimpleIndexBufferedRenderable.h"
#include "renderable/SimpleRenderable.h"
#include "shader/SimpleShaderObject.h"

struct VertexData: Vertex {
    glm::vec3 pos;
    glm::vec2 texCoords;
};

struct TestShaderData {
    VkDeviceAddress globalDataBufferAddress;
    VkDeviceAddress transformDataBufferAddress;
};

class TestShaderObject final: public SimpleShaderObject {
public:
    bool canObjectsTick() override;

    VertexInput getVertexInput() override;

    TestShaderObject(const std::string& vertexShaderPath, const std::string &fragmentShaderPath, const ShaderProperties &properties): SimpleShaderObject(vertexShaderPath, fragmentShaderPath, properties, {{VK_SHADER_STAGE_VERTEX_BIT, sizeof(TestShaderData), 0}}) {
    }

    void initRenderable(Renderable *renderable) override {
        if (auto* rend = dynamic_cast<SimpleIndexBufferedRenderable<TestShaderData>*>(renderable); rend != nullptr) {
            rend->transformBuffer = BufferManager<glm::mat4>{sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY, 0, 1};
        }
    }

    std::vector<Descriptor> getDescriptors() override {
        return {Descriptor {0, 0, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}};
    }

    void initShader() override {

    }

    void createBuffers() override {

    }
};



#endif //TESTSHADEROBJECT_H

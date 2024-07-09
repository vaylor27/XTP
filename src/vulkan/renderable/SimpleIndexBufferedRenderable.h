
#ifndef TESTRENDERABLE_H
#define TESTRENDERABLE_H
#include "../../../test/TestShaderObject.h"
#include "SimpleRenderable.h"


template <class T> class SimpleIndexBufferedRenderable final : public SimpleRenderable {
public:

    SimpleIndexBufferedRenderable(std::vector<T> vertices, std::vector<uint32_t> indices, std::shared_ptr<ShaderObject>& shader): shader(shader) {
        this->vertices = vertices;
        this->indices = indices;
    }

    std::vector<T> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<ShaderObject>& shader;

    std::shared_ptr<ShaderObject>& getShader() override {
        return shader;
    }

    AllocatedBuffer vertexBuffer = {};
    AllocatedBuffer indexBuffer = {};

    void remove() override {
        vertexBuffer.destroy();
        indexBuffer.destroy();
    }


    void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) override {
        vertexBuffer.bind(commandBuffer);
        indexBuffer.bind(commandBuffer);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    void createBuffers() override {
        vertexBuffer = createBufferWithData(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        indexBuffer = createBufferWithData(indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    void tick() override {

    }
};

#endif //TESTRENDERABLE_H

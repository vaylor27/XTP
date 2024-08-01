
#ifndef SIMPLEINDEXBUFFEREDRENDERABLE_H
#define SIMPLEINDEXBUFFEREDRENDERABLE_H

#include "SimpleRenderable.h"
#include "buffer/BufferManager.h"
#include "glm/glm.hpp"
#include "shader/SimpleShaderObject.h"


template <class T> class SimpleIndexBufferedRenderable : public SimpleRenderable {
public:
    BufferManager<glm::mat4> transformBuffer {};

    SimpleIndexBufferedRenderable(const std::shared_ptr<SimpleShaderObject>& shader, std::shared_ptr<Mesh> mesh, const glm::mat4 &initialTransform = {}):
        mesh(std::move(mesh)), shader(shader) {

        transformBuffer.bufferValue = initialTransform;
        transformBuffer.markBuffersDirty();
    }

    std::shared_ptr<Mesh> mesh;
    const std::shared_ptr<SimpleShaderObject>& shader;

    std::shared_ptr<ShaderObject> getShader() override {
        return shader;
    }

    void remove() override {
        transformBuffer.cleanUp();
    }

    bool mouseSelectable() override {
        return false;
    }

    virtual T getPushConstants(uint32_t frameIndex) = 0;

    void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) override {
        SimpleShaderObject::bindPushConstant(commandBuffer, getPushConstants(XTPVulkan::currentFrameIndex), shader.get());
        transformBuffer.onFrame(XTPVulkan::currentFrameIndex);
        getMesh()->getVertexBuffer().bind(commandBuffer);
        getMesh()->getIndexBuffer().bind(commandBuffer);

        vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1, 0, 0, 0);
    }


    std::shared_ptr<Mesh> getMesh() override {
        return mesh;
    }
};

#endif //SIMPLEINDEXBUFFEREDRENDERABLE_H

//
// Created by Viktor Aylor on 15/07/2024.git submodule update --recursive
//

#ifndef TESTRENDERABLE_H
#define TESTRENDERABLE_H
#include <memory>
#include <utility>

#include "TestMaterial.h"
#include "TestShaderObject.h"
#include "renderable/SimpleIndexBufferedRenderable.h"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"


class TestRenderable final : public SimpleIndexBufferedRenderable<TestShaderData> {
public:
    static std::shared_ptr<Material> TEST_MTL;

    TestRenderable(const std::shared_ptr<SimpleShaderObject> &shader, std::shared_ptr<Mesh> mesh)
        : SimpleIndexBufferedRenderable(shader, std::move(mesh)) {
    }

    std::shared_ptr<Material> getMaterial() override {
        if (TEST_MTL == nullptr) {
            TEST_MTL = std::shared_ptr<Material>(new TestMaterial());
        }
        return TEST_MTL;
    }

    void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) override {
        glm::mat4 mtx = glm::identity<glm::mat4>();
        mtx = rotate(mtx, glm::radians(0.0f), glm::vec3(1, 0, 0));
        mtx = rotate(mtx, glm::radians(0.0f), glm::vec3(0, 1, 0));
        mtx = rotate(mtx, glm::radians(0.0f), glm::vec3(0, 0, 1));
        mtx = translate(mtx, glm::vec3(0, 0, 0));
        mtx = scale(mtx, glm::vec3(1, 1, 1));

        transformBuffer.bufferValue = mtx;
        transformBuffer.markBuffersDirty();

        transformBuffer.onFrame(XTPVulkan::currentFrameIndex);
        getMesh()->getVertexBuffer().bind(commandBuffer);
        getMesh()->getIndexBuffer().bind(commandBuffer);


        SimpleShaderObject::bindPushConstant(commandBuffer, getPushConstants(XTPVulkan::currentFrameIndex), shader.get());
        vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1, 0, 0, 0);
    }

    TestShaderData getPushConstants(const uint32_t frameIndex) override {
        return TestShaderData {
            XTPVulkan::globalSceneDataBuffers[frameIndex].gpuAddress,
            transformBuffer.getBuffer(frameIndex)->gpuAddress
        };
    }

    void createBuffers() override {

    }
};



#endif //TESTRENDERABLE_H

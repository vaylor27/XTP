
#ifndef TESTMATERIAL_H
#define TESTMATERIAL_H
#include "Material.h"
#include "TestShaderObject.h"


class TestMaterial final : public Material {
    XTPVulkan::DescriptorSet testDescriptor {};
    AllocatedImage testImg = XTPVulkan::createImage("./assets/textures/meme.jpg", VK_FORMAT_R8G8B8A8_UNORM);
    bool hasInitialized = false;

    bool initialized() override {
        return hasInitialized;
    }

    void init(ShaderObject* shader) override {
        XTPVulkan::createSampler(testImg);
        if (auto* obj = dynamic_cast<TestShaderObject*>(shader); obj != nullptr) {
            testDescriptor = obj->createDescriptorSet(0);
        }
        testDescriptor.setData(testImg, 0);
        hasInitialized = true;
    }

    void prepareForRender(VkCommandBuffer commandBuffer, ShaderObject* object) override {
        testDescriptor.bind(commandBuffer, object);
    }

    void cleanUp() override {
        //The image is automatically destroyed, so we don't need to take care of it.
    }
};



#endif //TESTMATERIAL_H

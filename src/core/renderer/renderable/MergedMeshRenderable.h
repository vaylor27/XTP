//
// Created by Viktor Aylor on 15/07/2024.
//

#ifndef MERGEDMESHRENDERABLE_H
#define MERGEDMESHRENDERABLE_H
#include "Mesh.h"
#include "SimpleIndexBufferedRenderable.h"
#include "XTPVulkan.h"

class MergedMeshRenderable : SimpleRenderable {
public:
    void remove() override {
    }

    bool mouseSelectable() override {
        return false;
    }

    std::shared_ptr<Mesh> getMesh() override {
        return nullptr;
    }

    std::shared_ptr<Material> getMaterial() override {
        return material;
    }

    std::shared_ptr<ShaderObject> getShader() override {
        return shader;
    }

    std::vector<std::shared_ptr<Renderable>> renderables;
    std::shared_ptr<ShaderObject> shader;
    std::shared_ptr<Material> material;
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indexBuffer;

    MergedMeshRenderable(const std::shared_ptr<Material>& material, const std::shared_ptr<ShaderObject>& shader, const std::vector<std::shared_ptr<Renderable>>& renderables) {
        this->material = material;
        this->shader = shader;
        this->renderables = renderables;
        this->vertexBuffer = {};
        this->indexBuffer = {};
    }

    void createBuffers() override {
        XTPVulkan::mergeMeshes(renderables, vertexBuffer, indexBuffer);
    }

    void addRenderable(const std::shared_ptr<Renderable>& renderable) {
        XTPVulkan::destroyAllocatedBuffer(&vertexBuffer);
        XTPVulkan::destroyAllocatedBuffer(&indexBuffer);
        createBuffers();
    }

    void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) override {
        //TODO
    }
};



#endif //MERGEDMESHRENDERABLE_H


#ifndef RENDERABLE_H
#define RENDERABLE_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Mesh.h"

class Material;
class ShaderObject;

class Renderable {
public:
    virtual ~Renderable() = default;

    virtual void init() = 0;

    virtual void remove() = 0;

    virtual bool shouldRemove() {
        return false;
    }

    virtual bool hasRemoved() {
        return false;
    }

    virtual std::shared_ptr<ShaderObject> getShader() = 0;

    virtual bool hasInitialized() = 0;

    virtual bool mouseSelectable() = 0;

    virtual void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;

    virtual void tick() {} //Called on the tick thread

    virtual std::shared_ptr<Mesh> getMesh() = 0;

    virtual std::shared_ptr<Material> getMaterial() = 0;
};



#endif //RENDERABLE_H

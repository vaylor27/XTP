
#ifndef RENDERABLE_H
#define RENDERABLE_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "shader/ShaderObject.h"


class Renderable {
public:
    virtual ~Renderable() = default;

    virtual void init() = 0;

    virtual void remove() = 0;

    virtual bool shouldRemove() {
        return false;
    }

    virtual std::shared_ptr<ShaderObject>& getShader();;

    virtual bool hasInitialized() = 0;

    virtual void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;

    virtual void tick() {}
};



#endif //RENDERABLE_H

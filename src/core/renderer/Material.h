#ifndef MATERIAL_H
#define MATERIAL_H
#include "shader/ShaderObject.h"

class Material {
public:
    virtual ~Material() = default;

    virtual void cleanUp() {}

    virtual bool initialized() = 0;

    virtual void prepareForRender(VkCommandBuffer commandBuffer, ShaderObject* object) {}

    virtual void init(ShaderObject* shader) {}
};

#endif //MATERIAL_H

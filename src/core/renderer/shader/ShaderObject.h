
#ifndef SHADEROBJECT_H
#define SHADEROBJECT_H

#include "FileUtil.h"
#include "buffer/AllocatedBuffer.h"
#include "renderable/Renderable.h"

struct ShaderData {};

class ShaderObject {
public:
    virtual ~ShaderObject() = default;

    virtual void cleanUp() = 0;

    virtual bool canObjectsTick() = 0;

    virtual void prepareForRender(const VkCommandBuffer &commandBuffer) = 0;

    virtual void initRenderable(Renderable* renderable) = 0;

    virtual bool hasDeleted() = 0;

    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    virtual VkPipelineLayout getLayout() = 0;

    // ReSharper disable once CppPossiblyUninitializedMember
    ShaderObject(const std::string &vertexShaderPath,
                 const std::string &fragmentShaderPath
                 ) {

        this->vertexShaderPath = vertexShaderPath;
        this->fragmentShaderPath = fragmentShaderPath;
    }

    virtual void init() = 0;
};



#endif //SHADEROBJECT_H

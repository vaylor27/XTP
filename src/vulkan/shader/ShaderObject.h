
#ifndef SHADEROBJECT_H
#define SHADEROBJECT_H

#include <iostream>

#include "FileUtil.h"
#include <vulkan/vulkan.h>

struct ShaderData {};

class ShaderObject {
public:
    virtual ~ShaderObject() = default;

    virtual void cleanUp() = 0;

    virtual bool canObjectsTick() = 0;

    virtual void prepareForRender(const VkCommandBuffer &commandBuffer) = 0;

    std::string vertexShaderPath;
    std::string fragmentShaderPath;

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

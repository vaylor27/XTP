
#include "TestShaderObject.h"

#include <glm/glm.hpp>

#include "XTPVulkan.h"

bool TestShaderObject::canObjectsTick() {
    return true;
}

VertexInput TestShaderObject::getVertexInput() {
    VertexInput input {};

    VertexInputData posData {};
    VertexAttribute positionAttribute {
        0,
        SHADER_INPUT_VECTOR3F,
        offsetof(VertexData, pos)
    };
    VertexAttribute texCoordAttribute {
        2,
        SHADER_INPUT_VECTOR2F,
        offsetof(VertexData, texCoords)
    };
    posData.attributes.emplace_back(positionAttribute);
    posData.attributes.emplace_back(texCoordAttribute);
    posData.stride = sizeof(VertexData);

    input.vertexData.emplace_back(posData);

    return input;
}
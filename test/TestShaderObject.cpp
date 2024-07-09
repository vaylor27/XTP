
#include "TestShaderObject.h"

#include <glm/glm.hpp>

#include "XTPVulkan.h"

bool TestShaderObject::canObjectsTick() {
    return true;
}

VertexInput TestShaderObject::getVertexInput() {
    VertexInput input {};

    VertexData posData {};
    VertexAttribute positionAttribute {
        0,
        SHADER_INPUT_VECTOR2F,
        offsetof(Vertex, pos)
    };
    VertexAttribute colorAttribute {
        1,
        SHADER_INPUT_VECTOR3F,
        offsetof(Vertex, color)
    };
    posData.attributes.emplace_back(positionAttribute);
    posData.attributes.emplace_back(colorAttribute);
    posData.stride = sizeof(Vertex);

    input.vertexData.emplace_back(posData);

    return input;
}
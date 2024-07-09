
#ifndef TESTSHADEROBJECT_H
#define TESTSHADEROBJECT_H
#include <glm/glm.hpp>

#include "shader/SimpleShaderObject.h"

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

class TestShaderObject final: public SimpleShaderObject {
public:
    bool canObjectsTick() override;

    VertexInput getVertexInput() override;

    TestShaderObject(const std::string& vertexShaderPath, const std::string &fragmentShaderPath, const ShaderProperties &properties): SimpleShaderObject(vertexShaderPath, fragmentShaderPath, properties) {
    }
};



#endif //TESTSHADEROBJECT_H

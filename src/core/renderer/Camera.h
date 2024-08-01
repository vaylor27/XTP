//
// Created by Viktor Aylor on 10/07/2024.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/vec3.hpp>

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"


class Camera {
public:
    bool shouldUpdateViewMatrix = true;

    virtual ~Camera() = default;

    virtual glm::mat4 createViewMatrix() {
        auto viewMatrix = glm::identity<glm::mat4>();

        viewMatrix = rotate(viewMatrix, static_cast<float>(glm::radians(getPitch())), glm::vec3(1, 0, 0));
        viewMatrix = rotate(viewMatrix, static_cast<float>(glm::radians(getYaw())), glm::vec3(0, 1, 0));
        viewMatrix = rotate(viewMatrix, static_cast<float>(glm::radians(getRoll())), glm::vec3(0, 0, 1));

        const glm::vec3 invertedPos = -getPosition();

        viewMatrix = translate(viewMatrix, invertedPos);

        return viewMatrix;
    }

    static std::shared_ptr<Camera> camera;

    virtual glm::dvec3 getPosition() = 0;

    virtual double getPitch() = 0;

    virtual double getYaw() = 0;

    virtual double getRoll() = 0;

    virtual void updateCamera() = 0;
};



#endif //CAMERA_H

//
// Created by Viktor Aylor on 10/07/2024.
//

#ifndef SCREENPOSITIONRAY_H
#define SCREENPOSITIONRAY_H
#include <glm/glm.hpp>

#include "XTPWindowing.h"

//TODO

class ScreenPositionRay {
public:
    double posX, posY;

    ScreenPositionRay(const double posX, const double posY)
        : posX(posX),
          posY(posY) {
        auto size = XTPWindowing::getWindowSize();

        const auto x = static_cast<float>((2.0f * posX) / size.first - 1.0f);
        const auto y = static_cast<float>(1.0f - (2.0f * posY) / size.second);
        const auto z = 1.0f;  // We start with a point on the far plane
        glm::vec3 ndc(x, y, z);
        glm::vec4 clipCoords(ndc.x, ndc.y, -1.0f, 1.0f);
    }

};



#endif //SCREENPOSITIONRAY_H

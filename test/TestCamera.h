#ifndef TESTCAMERA_H
#define TESTCAMERA_H
#include "Camera.h"
#include "Options.h"
#include "XTPVulkan.h"
#include "XTPWindowing.h"


class TestCamera: public Camera {
public:
    glm::dvec3 position = {0, 0, 0};
    double pitch;
    double yaw;
    double roll;
    double angleAroundFocusPoint = 0;
    double distanceFromFocusPoint = 0;
    glm::dvec3 focusPoint = {0, 0, 0};
    glm::dvec3 focusPointRotation = {};

    glm::dvec3 getPosition() override {
        return position;
    }

    double getPitch() override {
        return pitch;
    }

    double getYaw() override {
        return yaw;
    }

    double getRoll() override {
        return roll;
    }

    void updateCamera() override {
        if (XTPWindowing::windowBackend->isMouseCaptured()) {
            calculateZoom();
            calculatePitch();
            calculateAngleAroundFocusPoint();

            const double horizontalDistance = getHorizontalDistanceFromFocusPoint();
            const double verticalDistance = getVerticalDistanceFromFocusPoint();
            calculateCameraPosition(horizontalDistance, verticalDistance);
            yaw = 180 - (focusPointRotation.y + angleAroundFocusPoint);
            shouldUpdateViewMatrix = true;
        }
    }

    [[nodiscard]] double getHorizontalDistanceFromFocusPoint() const {
        return distanceFromFocusPoint * cos(glm::radians(pitch));
    }

    [[nodiscard]] double getVerticalDistanceFromFocusPoint() const {
        return (distanceFromFocusPoint * sin(glm::radians(pitch)));
    }

    void calculateCameraPosition(const double horizontalDistance, const double verticalDistance) {
        const double theta = focusPointRotation.y + angleAroundFocusPoint;

        const double offsetX = horizontalDistance * sin(glm::radians(theta));
        const double offsetZ = horizontalDistance * cos(glm::radians(theta));

        position.x = focusPoint.x - offsetX;
        position.z = focusPoint.z - offsetZ;
        position.y = focusPoint.y + verticalDistance;
    }

    void calculateZoom() {
        const double zoomLevel = XTPWindowing::windowBackend->getScrollDelta().y * Options::scrollSensitivity;
        if (Options::reverseScrolling) {
            distanceFromFocusPoint += zoomLevel;
        } else {
            distanceFromFocusPoint -= zoomLevel;
        }
        distanceFromFocusPoint = glm::clamp(distanceFromFocusPoint, 0.0, 100.0);
    }

    virtual uint32_t getPitchChangeKeyBind() {
        return  -1;
    }

    virtual uint32_t getAngleChangeKeyBind() {
        return -1;
    }

    void calculatePitch() {
        if (getPitchChangeKeyBind() == -1 || XTPWindowing::windowBackend->isKeyPressed(getPitchChangeKeyBind())) {
            double pitchChange = XTPWindowing::windowBackend->getMouseDelta().y * Options::mouseYSensitivity;
            if (Options::reverseMouseY) {
                pitch += pitchChange;
            } else {
                pitch -= pitchChange;
            }
        }
        pitch = glm::clamp(pitch, -90.0, 90.0);
    }

    void calculateAngleAroundFocusPoint() {
        if (getAngleChangeKeyBind() == -1 || XTPWindowing::windowBackend->isKeyPressed(getAngleChangeKeyBind())) {
            const double angleChange = XTPWindowing::windowBackend->getMouseDelta().x * Options::mouseXSensitivity;
            if (Options::reverseAngleChange) {
                angleAroundFocusPoint += angleChange;
            } else {
                angleAroundFocusPoint -= angleChange;
            }
            while (angleAroundFocusPoint > 360) {
                angleAroundFocusPoint -= 360;
            }
        }
    }
};



#endif //TESTCAMERA_H


#ifndef XTPWINDOW_H
#define XTPWINDOW_H

#include <vulkan/vulkan_core.h>
#include "glm/glm.hpp"

class XTPWindowBackend {
public:
    bool framebufferResized;

    XTPWindowBackend() = default;

    virtual ~XTPWindowBackend() = default;

    virtual void createWindow() = 0;

    virtual void getWindowSize(int* width, int* height) = 0;

    virtual void setWindowSize(int width, int height) = 0;

    virtual void setMousePos(double x, double y) = 0;

    virtual void getMousePos(double* x, double* y) = 0;

    virtual bool shouldClose() = 0;

    virtual void beginFrame() = 0;

    virtual void endFrame() = 0;

    virtual void destroyWindow() = 0;

    virtual bool captureMouse() = 0;

    virtual bool releaseMouse(bool returnToLastReleasedPos) = 0;

    virtual bool isMouseCaptured() = 0;

    virtual void getFramebufferSize(int * width, int * height) = 0;

    virtual void waitForEvents() = 0;

    virtual void pollEvents() = 0;

    virtual bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;

    virtual const char **getRequiredInstanceExtensions(uint32_t& count) = 0;

    virtual bool isKeyPressed(uint32_t keyCode) = 0;

    virtual glm::dvec2 getMouseDelta() = 0;

    virtual glm::dvec2 getScrollDelta() = 0;
};



#endif //XTPWINDOW_H

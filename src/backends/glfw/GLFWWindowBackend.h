
#ifndef GLFW_WINDOW_BACKENDXTP_H
#define GLFW_WINDOW_BACKENDXTP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "Events.h"
#include "KeyPressEvent.h"
#include "XTPVulkan.h"
#include "XTPWindowing.h"
#include "XTPWindowBackend.h"

class GLFWWindowBackend final: public XTPWindowBackend {
public:

    static GLFWwindow* window;
    static std::unordered_map<uint32_t, bool> keys;
    static glm::dvec2 scrollDelta;
    static glm::dvec2 mouseDelta;
    static glm::dvec2 mousePos;
    //Does not get written to when mouse is captured.
    static glm::dvec2 lastReleasedMousePos;
    static bool initializedMousePos;

    GLFWWindowBackend() = default;

    ~GLFWWindowBackend() override = default;

    void getWindowSize(int* width, int* height) override {
        glfwGetWindowSize(window, width, height);
    }

    void endFrame() override {
        scrollDelta = {0, 0};
        mouseDelta = {0, 0};
    }

    void setWindowSize(const int width, const int height) override {
        glfwSetWindowSize(window, width, height);
    }

    bool shouldClose() override {
        return glfwWindowShouldClose(window);
    }

    void beginFrame() override {
    }

    void destroyWindow() override {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    static void initCallbacks() {
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            XTPWindowing::windowBackend->framebufferResized = true;
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            scrollDelta = glm::dvec2 {xoffset, yoffset};
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            if (!initializedMousePos) {
                mousePos = glm::dvec2 {xpos, ypos};
                initializedMousePos = true;
            }
            mouseDelta = mousePos - glm::dvec2 {xpos, ypos};
            mousePos = glm::dvec2 {xpos, ypos};
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            keys[button] = action == GLFW_REPEAT || action == GLFW_PRESS;
            Events::callFunctionOnAllEventsOfType<KeyPressEvent>([button, action, mods](auto event) {
                event->onKeyPressed(button, action, mods);
            });
        });

        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            keys[key] = action == GLFW_REPEAT || action == GLFW_PRESS;
            Events::callFunctionOnAllEventsOfType<KeyPressEvent>([key, action, mods](auto event) {
                event->onKeyPressed(key, action, mods);
            });
        });
    }

    void createWindow() override {
        if (XTPWindowing::data == nullptr) {
            throw std::runtime_error("Window Data Must Not Be Null!");
        }
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(XTPWindowing::data->getWidth(), XTPWindowing::data->getHeight(), XTPWindowing::data->getWindowTitle(), nullptr, nullptr);

        initCallbacks();
    }

    bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) override {
        return glfwCreateWindowSurface(instance, window, nullptr, surface) == VK_SUCCESS;
    }

    const char **getRequiredInstanceExtensions(uint32_t& count) override {
        return glfwGetRequiredInstanceExtensions(&count);
    }

    void getFramebufferSize(int *width, int *height) override {
        glfwGetFramebufferSize(window, width, height);
    }

    void waitForEvents() override {
        glfwWaitEvents();
    }

    void pollEvents() override {
        glfwPollEvents();
    }


    bool isKeyPressed(const uint32_t keyCode) override {
        return keys[keyCode];
    }

    glm::dvec2 getMouseDelta() override {
        return mouseDelta;
    }

    glm::dvec2 getScrollDelta() override {
        return scrollDelta;
    }

    bool captureMouse() override {
        if (isMouseCaptured()) {
            return false;
        }
        lastReleasedMousePos = mousePos;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
        return true;
    }

    bool releaseMouse(const bool returnToLastReleasedPos) override {
        if (!isMouseCaptured()) {
            return false;
        }
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (returnToLastReleasedPos) {
            setMousePos(lastReleasedMousePos.x, lastReleasedMousePos.y);
        }
        return true;
    }

    bool isMouseCaptured() override {
        return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    }

    void setMousePos(const double x, const double y) override {
        glfwSetCursorPos(window, x, y);
    }

    void getMousePos(double *x, double *y) override {
        glfwGetCursorPos(window, x, y);
    }
};



#endif //GLFW_WINDOW_BACKENDXTP_H

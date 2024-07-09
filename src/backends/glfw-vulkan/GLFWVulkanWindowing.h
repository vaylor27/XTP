
#ifndef GLFWVULKANWINDOWING_H
#define GLFWVULKANWINDOWING_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanWindow.h"
#include <iostream>

#include "XTPVulkan.h"
#include "XTPWindowing.h"


class GLFWVulkanWindowing final : public VulkanWindow {
public:

    static GLFWwindow* window;

    GLFWVulkanWindowing() = default;

    ~GLFWVulkanWindowing() override = default;

    std::pair<int, int> getWindowSize() override {
        int width;
        int height;

        glfwGetWindowSize(window, &width, &height);

        return {width, height};
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

    void createWindow() override {
        if (XTPWindowing::data == nullptr) {
            throw std::runtime_error("Window Data Must Not Be Null!");
        }
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(XTPWindowing::data->getWidth(), XTPWindowing::data->getHeight(), XTPWindowing::data->getWindowTitle(), nullptr, nullptr);

        glfwSetFramebufferSizeCallback(window, onFrameBufferResized);
    }

    static void onFrameBufferResized(GLFWwindow* window, int width, int height) {
        XTPWindowing::windowBackend->framebufferResized = true;
    }

    bool supportsRenderer(const char *str) override {
        return strcmp(str, "vulkan") == 0;
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
};



#endif //GLFWVULKANWINDOWING_H

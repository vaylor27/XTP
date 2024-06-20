
#ifndef GLFWVULKANWINDOWING_H
#define GLFWVULKANWINDOWING_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanWindow.h"
#include <iostream>

#include "XTPWindowing.h"

class GLFWVulkanWindowing final : public VulkanWindow {
public:

    static void* window;

    GLFWVulkanWindowing() = default;

    ~GLFWVulkanWindowing() override = default;

    std::pair<int, int> getWindowSize() override {
        int width;
        int height;

        glfwGetWindowSize(static_cast<GLFWwindow *>(window), &width, &height);

        return {width, height};
    }

    void setWindowSize(int width, int height) override {
        glfwSetWindowSize(static_cast<GLFWwindow *>(window), width, height);
    }

    bool shouldClose() override {
        return glfwWindowShouldClose(static_cast<GLFWwindow *>(window));
    }

    void beginFrame() override {
        glfwPollEvents();
    }

    void destroyWindow() override {
        glfwDestroyWindow(static_cast<GLFWwindow *>(window));
        glfwTerminate();
    }

    void createWindow() override {
        if (XTPWindowing::data == nullptr) {
            throw std::runtime_error("Window Data Must Not Be Null!");
        }
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(XTPWindowing::data->getWidth(), XTPWindowing::data->getHeight(), XTPWindowing::data->getWindowTitle(), nullptr, nullptr);
    }

    bool supportsRenderer(const char *str) override {
        return strcmp(str, "vulkan") == 0;
    }

    bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) override {
        return glfwCreateWindowSurface(instance, static_cast<GLFWwindow *>(window), nullptr, surface) == VK_SUCCESS;
    }

    const char **getRequiredInstanceExtensions(uint32_t& count) override {
        return glfwGetRequiredInstanceExtensions(&count);
    }
};



#endif //GLFWVULKANWINDOWING_H

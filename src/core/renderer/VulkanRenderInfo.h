
#ifndef VULKANRENDERDATA_H
#define VULKANRENDERDATA_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "SimpleLogger.h"
#include "glm/ext/matrix_clip_space.hpp"

class VulkanRenderInfo {
public:
    static VulkanRenderInfo* INSTANCE;


#ifdef NDEBUG
    static constexpr bool ISDEBUG = false;
#else
    static constexpr bool ISDEBUG = true;
#endif

    SimpleLogger* validationLogger = new SimpleLogger("Vulkan Validation Layers", TRACE);
    
    virtual ~VulkanRenderInfo() = default;

    virtual VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() {
        return {.samplerAnisotropy = true};
    }

    virtual std::vector<std::string> getRequiredDeviceExtensions() {
        return {"VK_KHR_dynamic_rendering", "VK_KHR_buffer_device_address", "VK_KHR_shader_non_semantic_info"};
    }

    virtual uint32_t getTickRateMilis() {
        return 0;
    }

    virtual std::vector<std::string> getRequiredInstanceExtensions() {
        return {};
    }

    virtual std::vector<const char *> getValidationLayers() {
        return {"VK_LAYER_KHRONOS_validation"};
    }

    virtual bool isDeviceSuitable(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceProperties & deviceProperties, const VkPhysicalDeviceFeatures & deviceFeatures, const std::vector<std::string> & vector) {
        return true;
    }

    virtual uint32_t getDeviceScore(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceProperties & deviceProperties, const VkPhysicalDeviceFeatures & deviceFeatures, const std::vector<std::string> & vector) {
        uint32_t score {};

        score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000: 0;

        return score;
    }

    virtual bool isQueueFamilySuitable(const VkQueueFamilyProperties& queueFamily) {
        return true;
    }

    virtual uint32_t getQueueFamilyGraphicsScore(const VkQueueFamilyProperties& queueFamily) {
        return 1;
    }

    virtual uint32_t getQueueFamilyPresentScore(const VkQueueFamilyProperties& queueFamily) {
        return 1;
    }

    virtual void onVulkanDebugMessage(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkFlags uint32, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData, const std::string & message) {
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                validationLogger->logTrace(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                validationLogger->logInformation(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                validationLogger->logWarning(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                validationLogger->logError(message, false);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
                break;
        }
    }

    virtual const char* getApplicationName() {
        return "Default XTP App";
    }

    virtual uint32_t getApplicationVersion() {
        return VK_MAKE_VERSION(1, 0, 0);
    }

    virtual VkClearColorValue getClearColor() {
        return {{0.0f, 0.0f, 0.0f, 1.0f}};
    }

    virtual uint32_t getMaxFramesInFlight() {
        return 2;
    }

    virtual float getFOV() {
        return 90;
    }

    virtual float getZNear() {
        return 0.1f;
    }

    virtual float getZFar() {
        return 100.0f;
    }
};


#endif //VULKANRENDERDATA_H

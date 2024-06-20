
#ifndef VULKANRENDERDATA_H
#define VULKANRENDERDATA_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "XTPRenderInfo.h"
#include "XTPVulkan.h"


class VulkanRenderInfo: public XTPRenderInfo {
public:
    ~VulkanRenderInfo() override = default;

    virtual VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() {
        return {};
    }

    virtual std::vector<std::string> getRequiredDeviceExtensions() {
        return {};
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
                XTPVulkan::validationLogger->logTrace(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                XTPVulkan::validationLogger->logInformation(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                XTPVulkan::validationLogger->logWarning(message);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                XTPVulkan::validationLogger->logError(message);
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

};



#endif //VULKANRENDERDATA_H

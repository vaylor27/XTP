
#ifndef XTP_XTPVULKAN_H
#define XTP_XTPVULKAN_H

#include "XTP.h"
#include <set>

#include "VkFormatParser.h"
#include "XTPRenderBackend.h"
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    uint32_t graphicsScore {0};
    uint32_t gresentScore {0};

    [[nodiscard]] bool isComplete() const;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class XTPVulkan: public XTPRenderBackend {
public:

    XTPVulkan() = default;


    void initializeBackend() override;

    void cleanUp() override;

    static SimpleLogger* validationLogger;
    static SimpleLogger* logger;

    static bool useValidationLayers;


private:
    static std::string makeListOf(const std::vector<const char*>& elements);

    static std::vector<VkExtensionProperties> _availableInstanceExtensions;
    static std::vector<VkExtensionProperties> _availableDeviceExtensions;
    static std::vector<VkLayerProperties> _availableValidationLayers;

    static std::optional<VkInstance> _vkInstance;
    static std::optional<VkDebugUtilsMessengerEXT> _debugUtilsMessenger;
    static std::optional<VkPhysicalDevice> _gpu;
    static std::optional<VkDevice> _device;
    static std::optional<QueueFamilyIndices> _indices;
    static std::optional<VkQueue> _graphicsQueue;
    static std::optional<VkQueue> _presentQueue;
    static std::optional<VkSurfaceKHR> _surface;


    static void printAvailableDeviceExtensions();

    static VkSurfaceKHR createSurface();

    static VkDevice createLogicalDevice();

    static VkSurfaceKHR getSurface();

    static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device) {
        SwapChainSupportDetails details {};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, getSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, getSurface(), &formatCount, nullptr);

        if (formatCount > 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, getSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, getSurface(), &presentModeCount, nullptr);

        if (formatCount > 0) {
            details.presentModes.resize(formatCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, getSurface(), &formatCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats) {
        XTP::getLogger()->logDebug("Choosing Swap Surface Format!");
        for (const VkSurfaceFormatKHR availableFormat : availableFormats) {
            XTP::getLogger()->logDebug("Found Surface Format " + VkFormatParser::toStringVkFormat(availableFormat.format));
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

            }
        }
    }

    static std::vector<VkExtensionProperties> getVkDeviceExtensionInfo();

    static std::vector<std::string> getVkDeviceExtensionNames(VkPhysicalDevice device = nullptr);

    static uint32_t getVkDeviceExtensionCount(VkPhysicalDevice device);

    static QueueFamilyIndices getIndices();

    static VkQueue getGraphicsQueue();

    static VkQueue getPresentQueue();

    static VkDevice getDevice();

    // ReSharper disable once InconsistentNaming
    static VkPhysicalDevice getGPU();

    static VkPhysicalDevice pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices);

    static std::vector<VkPhysicalDevice> getAllPhysicalDevices();

    static uint32_t getDeviceScore(VkPhysicalDevice physicalDevice, QueueFamilyIndices &queueFamilyIndices,
                                   VkPhysicalDeviceProperties &physicalDeviceProperties);

    static bool isDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features,
                                 const std::vector<std::string>& extensions, const QueueFamilyIndices &indices);

    static void getDeviceInfo(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties &properties, VkPhysicalDeviceFeatures &features);


    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const std::string &deviceName);

    static std::string getQueueCaps(const VkQueueFamilyProperties &queueFamily);

    static VkInstance getVkInstance();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);


    static VkDebugUtilsMessengerEXT getDebugMessenger();

    static VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();

    static VkInstance createVulkanInstance();

    static void printAvailableVulkanProperties();

    static bool checkForValidationLayerSupport();

    static bool ensureAllValidationLayersPresent(const std::vector<VkLayerProperties>& availableLayers);

    static std::string getLayerName(const VkLayerProperties &layerProperties);

    static std::vector<const char*> findAllVulkanInstanceExtensions(VkInstanceCreateFlags &flags);

    static std::string getExtensionName(const VkExtensionProperties &extension);

    static void setupAdditionalVulkanInstanceExtensions(std::vector<const char*> &enabledInstanceExtensions, VkInstanceCreateFlags &flags);

    static std::vector<VkExtensionProperties> getVkInstanceExtensionInfo();

    static uint32_t getVkInstanceExtensionCount();

    static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
};


#endif //XTP_XTPVULKAN_H

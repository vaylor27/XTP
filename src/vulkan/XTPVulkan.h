
#ifndef XTP_XTPVULKAN_H
#define XTP_XTPVULKAN_H

#include<vk_mem_alloc.h>

#include <set>
#include <cstdint> // Necessary for uint32_t

#include "shader/ShaderObject.h"
#include "VkFormatParser.h"

#include "VulkanRenderInfo.h"
#include "renderable/Renderable.h"

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

class XTPVulkan final {
public:

    static constexpr const char* engineName = "VQT Engine";
    static constexpr uint32_t engineVersion = VK_MAKE_VERSION(0, 0, 1);

#ifdef NDEBUG
    static constexpr bool ISDEBUG = false;
#else
    static constexpr bool ISDEBUG = true;
#endif

    static std::vector<VkImage> swapchainImages;
    static VkFormat swapchainImageFormat;
    static VkExtent2D swapchainExtent;
    static std::vector<VkImageView> swapchainImageViews;
    static std::vector<VkFramebuffer> swapchainFramebuffers;
    static std::vector<VkExtensionProperties> availableInstanceExtensions;
    static std::vector<VkExtensionProperties> availableDeviceExtensions;
    static std::vector<VkLayerProperties> availableValidationLayers;

    static VkInstance instance;
    static VkDebugUtilsMessengerEXT debugUtilsMessenger;
    static VkPhysicalDevice gpu;
    static VkDevice device;
    static QueueFamilyIndices queueIndices;
    static VkQueue graphicsQueue;
    static std::unordered_map<const char*, VkRenderPass> renderPasses;
    static VkQueue presentQueue;
    static VkSurfaceKHR surface;
    static VkSwapchainKHR swapchain;
    static VkCommandPool commandPool;
    static std::vector<VkCommandBuffer> commandBuffers;
    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> renderFinishedSemaphores;
    static std::vector<VkFence> inFlightFences;
    static uint32_t currentFrameIndex;
    static bool initialized;
    static std::unordered_map<std::shared_ptr<ShaderObject>, std::vector<std::shared_ptr<Renderable>>> toRender;
    static VmaAllocator allocator;

    static void drawFrame();

    static void cleanupSwapchain();

    static void recreateSwapchain();

    static VkCommandPool createCommandPool();

    static void createSyncObjects();

    static VmaAllocator createAllocator();

    static void init();

    static void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static std::vector<VkCommandBuffer> createCommandBuffers();

    static void cleanUp();

    static VkSwapchainKHR createSwapchain();

    static void createFramebuffers();

    static VkRenderPass getMainRenderPass();

    static void render();

    static bool hasInitialized();

    static SimpleLogger* logger;

    static bool useValidationLayers;

    static std::string makeListOf(const std::vector<const char*>& elements);


    static void addShader(const std::shared_ptr<ShaderObject> &shader);

    static void addRenderable(const std::shared_ptr<Renderable> &renderable);

    static std::vector<VkImageView> createImageViews();

    static void printAvailableDeviceExtensions();

    static VkSurfaceKHR createSurface();

    static VkDevice createLogicalDevice();

    static VkRenderPass createRenderPass();

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static std::vector<VkExtensionProperties> getVkDeviceExtensionInfo();

    static std::vector<std::string> getVkDeviceExtensionNames(VkPhysicalDevice device = nullptr);

    static uint32_t getVkDeviceExtensionCount(VkPhysicalDevice device);

    static VkPhysicalDevice pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices);

    static std::vector<VkPhysicalDevice> getAllPhysicalDevices();

    static uint32_t getDeviceScore(VkPhysicalDevice physicalDevice, QueueFamilyIndices &queueFamilyIndices,
                                   VkPhysicalDeviceProperties &physicalDeviceProperties);

    static bool isDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features,
                                 const std::vector<std::string>& extensions, const QueueFamilyIndices &indices);

    static void getDeviceInfo(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties &properties, VkPhysicalDeviceFeatures &features);


    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const std::string &deviceName = "", bool printDebug = false);

    static std::string getQueueCaps(const VkQueueFamilyProperties &queueFamily);

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

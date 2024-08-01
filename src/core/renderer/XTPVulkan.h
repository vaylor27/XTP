
#ifndef XTP_XTPVULKAN_H
#define XTP_XTPVULKAN_H
#define ImDrawIdx unsigned int
#include <vk_mem_alloc.h>

#include <set>
#include <cstdint> // Necessary for uint32_t
#include "AllocatedImage.h"
#include "descriptor_allocator.h"
#include "buffer/AllocatedBuffer.h"
#include "Material.h"
#include "shader/ShaderObject.h"
#include "VulkanRenderInfo.h"
#include "glm/glm.hpp"
#include "renderable/Renderable.h"

struct AllocatedImage;

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

struct SceneRenderData {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
};

class XTPVulkan final {
public:

    static constexpr const char* engineName = "VQT Engine";
    static constexpr uint32_t engineVersion = VK_MAKE_VERSION(0, 0, 1);

#ifndef NDEBUG
#define ISDEBUG
#endif

    static std::vector<VkImage> swapchainImages;
    static VkFormat swapchainImageFormat;
    static VkExtent2D swapchainExtent;
    static std::vector<VkImageView> swapchainImageViews;
    static std::vector<VkFramebuffer> swapchainFramebuffers;
    static std::vector<VkExtensionProperties> availableInstanceExtensions;
    static std::vector<VkExtensionProperties> availableDeviceExtensions;
    static std::vector<VkLayerProperties> availableValidationLayers;
    static std::vector<bool> i;
    static std::vector<uint32_t> indices;
    static AllocatedImage errorTexure;
    static std::unique_ptr<vke::DescriptorAllocatorPool> allocatorPool;
    static bool initializedErrorTex;

    static glm::mat4 projectionMatrix;
    static glm::mat4 viewMatrix;
    static std::vector<AllocatedBuffer> globalSceneDataBuffers;
    static std::vector<bool> doesSceneBufferNeedToBeUpdated;
    static bool shouldUpdateProjectionMatrix;

    static VkInstance instance;
#ifdef ISDEBUG
    static VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
#ifdef XTP_USE_IMGUI_UI
    static VkDescriptorPool imguiPool;
#endif
#ifdef XTP_USE_ADVANCED_TIMING
    static VkQueryPool timeQueryPool;
    static std::vector<uint64_t> renderTimeNanos;
    static std::vector<bool> timeQueryInitialized;
#endif
    static VkPhysicalDevice gpu;
    static VkDevice device;
    static QueueFamilyIndices queueIndices;
    static uint32_t mostRecentFrameRendered;
    static VkQueue graphicsQueue;
    static VkQueue presentQueue;
    static VkRenderPass renderPass;
    static VkSurfaceKHR surface;
    static VkSwapchainKHR swapchain;
    static VkCommandPool commandPool;
    static std::vector<VkCommandBuffer> commandBuffers;
    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> renderFinishedSemaphores;
    static std::vector<VkFence> inFlightFences;
    static uint32_t currentFrameIndex;
    static bool initialized;
    static std::vector<AllocatedBuffer> buffers;
    static std::unordered_map<std::shared_ptr<ShaderObject>, std::unordered_map<std::shared_ptr<Material>, std::vector<std::shared_ptr<Renderable>>>> toRender;
    static VmaAllocator allocator;
    static VkPhysicalDeviceProperties gpuProperties;
    static std::vector<AllocatedImage> allLoadedImages;
    static std::vector<VkSampler> samplers;
    static AllocatedImage depthImage;

    static void drawFrame();

    static void cleanupSwapchain();

    static void recreateSwapchain();

    static VkCommandPool createCommandPool();

    static void createSyncObjects();

    static VmaAllocator createAllocator();

    static VkDescriptorPool initImgui();

    static VkQueryPool createTimeQueryPool();


    struct DescriptorSet {
        VkDescriptorSet set;
        VkDescriptorType type;

        void setData(const AllocatedBuffer &buffer, uint32_t binding, uint32_t arrayElement = 0, uint32_t offset = 0, uint32_t size = -1) const;

        void setData(AllocatedImage image, uint32_t binding, uint32_t arrayElement = 0) const;

        void bind(VkCommandBuffer commandBuffer, ShaderObject* shader) const;
    };

    static DescriptorSet createDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, ShaderObject* shader, VkDescriptorType type);

    static AllocatedImage createErrorTex();

    static void createImageView(AllocatedImage &image, VkFormat format, VkImageAspectFlags aspectFlags);

    static AllocatedImage createImage(void *pixels, VkDeviceSize imageSize, VkFormat imageFormat, uint32_t width, uint32_t height);

    static void createSampler(AllocatedImage& image, VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR, VkSamplerAddressMode
                              addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkSamplerAddressMode
                              addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT, bool useAnisotropy = true, float maxAnisotropy = -1,
                              VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK);

    static AllocatedImage createImage(const char *path, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);


    static AllocatedImage createDepthImage();

    static AllocatedImage createImage(uint32_t width, uint32_t height, VkFormat imageFormat, VkImageUsageFlags usage, VkImageAspectFlags
                                      aspectFlags);

    static void init();

    static glm::mat4 calculateProjectionMatrix(float fov, float zNear, float zFar);

    static void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex);

#ifdef XTP_USE_GLTF_LOADING
    //TODO: FINISH GLTF LOADING
    static void loadGltfModel(bool isBinaryGltf, const char* path, const char* sceneToLoad = nullptr);
#endif

    static std::vector<VkCommandBuffer> createCommandBuffers();

    static void cleanUp();

    static VkSwapchainKHR createSwapchain();

    static void createFramebuffers();

    static void updateGlobalMatrices();

    static void render();

    static bool hasInitialized();

    static SimpleLogger* logger;

    static uint64_t fetchRenderTimeNanos();

    static std::string makeListOf(const std::vector<const char*>& elements);

    static void addShader(const std::shared_ptr<ShaderObject> &shader);

    static void addMaterial(const std::shared_ptr<Material> &material, std::shared_ptr<ShaderObject> &shader);

    static void addRenderable(const std::shared_ptr<Renderable> &renderable);

    static std::vector<VkImageView> createImageViews();

    static void printAvailableDeviceExtensions();

    static VkSurfaceKHR createSurface();

    static void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

    template <class T> static AllocatedBuffer createSimpleBuffer(std::vector<T> data, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, bool record = true);

    struct Mesh {
        uint32_t firstIndex;
        uint32_t firstVertex;
    };


    [[nodiscard]] static AllocatedBuffer createSimpleBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, bool record = true);

    static void destroyAllocatedBuffer(AllocatedBuffer* buffer);

    static void transitionImageLayout(VkCommandBuffer cmd, AllocatedImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    static void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


    static void destroyAllocatedImage(AllocatedImage *image);

    template <class T> [[nodiscard]] static AllocatedBuffer createBufferWithDataStaging(const std::vector<T>& data, VkBufferUsageFlagBits usage) {
        AllocatedBuffer buf {};
        const VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        buf = createSimpleBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        AllocatedBuffer staging = createSimpleBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, false);
        void* map = staging.info.pMappedData;
        memcpy(map, data.data(), staging.info.size);

        copyBuffer(staging, buf, bufferSize);

        destroyAllocatedBuffer(&staging);

        return buf;
    }


    static void copyBuffer(AllocatedBuffer srcBuffer, AllocatedBuffer dstBuffer, VkDeviceSize size);

    static VkDevice createLogicalDevice();

    static VkRenderPass createRenderPass();

    static void mergeMeshes(const std::vector<std::shared_ptr<Renderable>> &renderables, AllocatedBuffer &mergedVertexBuffer, AllocatedBuffer
                            &mergedIndexBuffer);

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

    static std::vector<std::string> findAllVulkanInstanceExtensions(VkInstanceCreateFlags &flags);

    static std::string getExtensionName(const VkExtensionProperties &extension);

    static void setupAdditionalVulkanInstanceExtensions(std::vector<std::string> &enabledInstanceExtensions, VkInstanceCreateFlags &flags);

    static std::vector<VkExtensionProperties> getVkInstanceExtensionInfo();

    static uint32_t getVkInstanceExtensionCount();

    static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
};


#endif //XTP_XTPVULKAN_H

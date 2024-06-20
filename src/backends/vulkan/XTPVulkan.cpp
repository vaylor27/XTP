#include "XTPVulkan.h"

#include "VulkanRenderInfo.h"
#include "VulkanWindow.h"
#include "XTPRendering.h"
#include "XTPWindowBackend.h"
#include "XTPWindowing.h"

std::optional<VkInstance> XTPVulkan::_vkInstance;
std::optional<VkQueue> XTPVulkan::_presentQueue;
std::optional<VkQueue> XTPVulkan::_graphicsQueue;
bool XTPVulkan::useValidationLayers;
std::optional<VkDebugUtilsMessengerEXT> XTPVulkan::_debugUtilsMessenger;
std::vector<VkExtensionProperties> XTPVulkan::_availableDeviceExtensions;
std::vector<VkLayerProperties> XTPVulkan::_availableValidationLayers;
std::vector<VkExtensionProperties> XTPVulkan::_availableInstanceExtensions;
std::optional<VkPhysicalDevice> XTPVulkan::_gpu;
std::optional<VkDevice> XTPVulkan::_device;
std::optional<QueueFamilyIndices> XTPVulkan::_indices;
std::optional<VkSurfaceKHR> XTPVulkan::_surface;
SimpleLogger* XTPVulkan::validationLogger = new SimpleLogger("Vulkan Validation Layers", INFORMATION);
SimpleLogger* XTPVulkan::logger = new SimpleLogger("XTP Builtin Vulkan Renderer", XTP::DEBUG ? DEBUG: INFORMATION);

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value() && graphicsScore > 0 && gresentScore > 0;
}

void XTPVulkan::initializeBackend() {
    if (!XTPWindowing::windowBackend->supportsRenderer("vulkan")) {
        logger->logCritical("Window Backend Must Support Vulkan!");
    }
    if constexpr (!std::is_base_of_v<XTPRenderInfo, VulkanRenderInfo>) {
        logger->logCritical("Render Info Is Not Vulkan Render Data!");
    }

    logger->logInformation("Initializing Vulkan!");
    useValidationLayers = XTP::DEBUG;

    logger->logDebug("Getting Vulkan Instance Extension Info");
    _availableInstanceExtensions = getVkInstanceExtensionInfo();
    printAvailableVulkanProperties();

    _vkInstance = createVulkanInstance();
    if (useValidationLayers) {
        _debugUtilsMessenger = getDebugMessenger();
    }

    _surface = createSurface();

    QueueFamilyIndices indices;
    _gpu = pickPhysicalDevice(indices);
    _indices = indices;

    logger->logDebug("Getting Vulkan Device Extension Info");
    _availableDeviceExtensions = getVkDeviceExtensionInfo();
    printAvailableDeviceExtensions();

    _device = createLogicalDevice();
    logger->logInformation("Finished Initializing Vulkan!");
}

void XTPVulkan::cleanUp() {
    logger->logDebug("Cleaning Up Vulkan");
    if (useValidationLayers) {
        logger->logDebug("Destroying Debug Utils Messenger");
        destroyDebugUtilsMessengerEXT(getVkInstance(), _debugUtilsMessenger.value_or(nullptr), nullptr);
    }

    logger->logDebug("Destroying Vulkan Surface");
    vkDestroySurfaceKHR(getVkInstance(), getSurface(), nullptr);
    logger->logDebug("Destroying Vulkan Device");
    vkDestroyDevice(getDevice(), nullptr);
    logger->logDebug("Destroying Vulkan Instance");
    vkDestroyInstance(getVkInstance(), nullptr);
    logger->logDebug("Destroying Vulkan");
    delete validationLogger;
}

std::string XTPVulkan::makeListOf(const std::vector<const char *>& elements) {
    std::string str;
    int i = 0;
    for (const char* element : elements) {
        i++;
        str.append(element);
        if (i != elements.size()) {
            str.append(", ");
        }
    }
    return str;
}

void XTPVulkan::printAvailableDeviceExtensions() {
    logger->logDebug("Found " + std::to_string(_availableDeviceExtensions.size()) + " Vulkan Device Extensions!");
    for (VkExtensionProperties properties: _availableDeviceExtensions) {
        logger->logDebug("Found Vulkan Device Extension '" + std::string(properties.extensionName) + "'!");
    }
}

// ReSharper disable once CppNotAllPathsReturnValue
VkSurfaceKHR XTPVulkan::createSurface() {
    VkSurfaceKHR surface;
    logger->logDebug("Creating Vulkan Surface");
    if (reinterpret_cast<VulkanWindow*>(XTPWindowing::windowBackend)->createVulkanSurface(getVkInstance(), &surface)) return surface;

    logger->logCritical("Unable To Create Vulkan Surface");
}

VkDevice XTPVulkan::createLogicalDevice() {

    logger->logDebug("Creating Vulkan Device Queue Create Info");
    float queuePriority = 1;
    const std::set uniqueQueueFamilies = {getIndices().graphicsFamily.value(), getIndices().presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> infos(uniqueQueueFamilies.size());

    uint32_t i = 0;
    for (const uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = static_cast<uint32_t>(queueFamily);
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;

        infos[i] = info;
        i++;
    }


    logger->logDebug("Creating Vulkan Device Create Info");
    std::vector<const char*> layers;
    std::vector<const char*> deviceExtensions;

    std::vector<const char *> enabledDeviceExtensions = {};

    for (const auto [extensionName, specVersion] : _availableDeviceExtensions) {
        if (strcmp(extensionName, "VK_KHR_portability_subset") == 0) {
            enabledDeviceExtensions = {"VK_KHR_portability_subset"};
        }
    }

    enabledDeviceExtensions.emplace_back("VK_KHR_swapchain");

    for (std::string extension : dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getRequiredDeviceExtensions()) {
        enabledDeviceExtensions.emplace_back(extension.data());
    }

    for (std::string enabledDeviceExtension: enabledDeviceExtensions) {
        bool isPresent = false;
        for (const auto [extensionName, specVersion]: getVkDeviceExtensionInfo()) {
            if (extensionName == enabledDeviceExtension) {
                isPresent = true;
                break;
            }
        }

        if (!isPresent) {
            logger->logCritical("Unable To Find The Required Device Extension '" + enabledDeviceExtension + "'!");
        }
    }



    const VkPhysicalDeviceFeatures features = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getPhysicalDeviceFeatures();

    VkDeviceCreateInfo deviceCreateInfo {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = infos.data();
    deviceCreateInfo.queueCreateInfoCount = infos.size();
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensions.size();

    // For Compatibility With Older Vulkan Implementations That Distinguished Between Instance And Device Validation Layers
    const std::vector<const char*> enabledLayers = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getValidationLayers();

    if (useValidationLayers) {
        deviceCreateInfo.enabledLayerCount = enabledLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }




    VkDevice device;
    logger->logDebug("Creating Vulkan Device");
    if (const VkResult result = vkCreateDevice(getGPU(), &deviceCreateInfo, nullptr, &device); result != VK_SUCCESS) {
        logger->logCritical("Unable To Create Vulkan Logical Device");
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, getIndices().graphicsFamily.value_or(0), 0, &graphicsQueue);
    _graphicsQueue = graphicsQueue;


    VkQueue presentQueue;
    vkGetDeviceQueue(device, getIndices().presentFamily.value_or(0), 0, &presentQueue);
    _presentQueue = presentQueue;

    return device;
}

// ReSharper disable once CppNotAllPathsReturnValue
VkSurfaceKHR XTPVulkan::getSurface() {
    if (_surface.has_value()) return _surface.value();
    logger->logError("Attempted To Access Null Surface. Call XTPVulkan::initVulkan() Before Calling This Function.!");
}

std::vector<VkExtensionProperties> XTPVulkan::getVkDeviceExtensionInfo() {
    VkPhysicalDevice device = getGPU();

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> properties(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, properties.data());

    return properties;
}

std::vector<std::string> XTPVulkan::getVkDeviceExtensionNames(VkPhysicalDevice device) {
    if (device == nullptr) {
        device = getGPU();
    }

    uint32_t extensionCount = getVkDeviceExtensionCount(device);

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensionProperties.data());

    std::vector<std::string> propertiesArray(extensionCount);

    for (int i = 0; i < extensionCount; i++) {
        const auto [extensionName, specVersion] = extensionProperties[i];
        propertiesArray[i] = (extensionName);
    }

    return propertiesArray;
}

uint32_t XTPVulkan::getVkDeviceExtensionCount(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    return extensionCount;
}

// ReSharper disable once CppNotAllPathsReturnValue
QueueFamilyIndices XTPVulkan::getIndices() {
    if (_indices.has_value()) return _indices.value();
    logger->logError("Attempted To Access A Null Queue Family Indices. Call XTPVulkan::initVulkan() Before Calling This Function.!");

}

// ReSharper disable once CppNotAllPathsReturnValue
VkQueue XTPVulkan::getGraphicsQueue() {
    if (_graphicsQueue.has_value()) return _graphicsQueue.value();
    logger->logError("Attempted To Access A Null Graphics Queue. Call XTPVulkan::initVulkan() Before Calling This Function.!");

}

// ReSharper disable once CppNotAllPathsReturnValue
VkQueue XTPVulkan::getPresentQueue() {
    if (_presentQueue.has_value()) return _presentQueue.value();
    logger->logError("Attempted To Access A Null Present Queue. Call XTPVulkan::initVulkan() Before Calling This Function.!");

}

// ReSharper disable once CppNotAllPathsReturnValue
VkDevice XTPVulkan::getDevice() {
    if (_device.has_value()) return _device.value();
    logger->logError("Attempted To Access A Null Logical Device. Call XTPVulkan::initVulkan() Before Calling This Function.!");

}

// ReSharper disable once CppNotAllPathsReturnValue
VkPhysicalDevice XTPVulkan::getGPU() {
    if (_gpu.has_value()) return _gpu.value();
    logger->logError("Attempted To Access A Null Physical Device. Call XTPVulkan::initVulkan() Before Calling This Function.!");

}

VkPhysicalDevice XTPVulkan::pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices) {
    logger->logDebug("Picking Physical Device");
    queueFamilyIndices = {};
    std::optional<VkPhysicalDevice> device;
    uint32_t deviceScore = 0;

    const std::vector<VkPhysicalDevice> devices = getAllPhysicalDevices();

    bool foundSuitablePhysicalDevice = false;

    for (VkPhysicalDevice physicalDevice: devices) {
        QueueFamilyIndices indices;
        VkPhysicalDeviceProperties deviceProperties;
        const uint32_t score = getDeviceScore(physicalDevice, indices, deviceProperties);

        if (score <= deviceScore) continue;

        device = physicalDevice;
        deviceScore = score;
        queueFamilyIndices = indices;
        foundSuitablePhysicalDevice = true;
        logger->logDebug("Found New Best Physical Device '" + std::string(deviceProperties.deviceName) + "', Using This Device!");
    }

    if (foundSuitablePhysicalDevice && deviceScore != 0 && device.has_value()) return device.value();

    logger->logCritical("Unable To Find A Suitable GPU!");
    throw std::runtime_error("No Suitable GPU Was Found!");
}

std::vector<VkPhysicalDevice> XTPVulkan::getAllPhysicalDevices() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(getVkInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        logger->logCritical("Unable To Find A GPU With Vulkan Support!");
        throw std::runtime_error("No GPU With Vulkan Support Was Found!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(getVkInstance(), &deviceCount, devices.data());

    return devices;
}

uint32_t XTPVulkan::getDeviceScore(VkPhysicalDevice physicalDevice, QueueFamilyIndices &queueFamilyIndices,
                                   VkPhysicalDeviceProperties &physicalDeviceProperties) {

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    getDeviceInfo(physicalDevice, deviceProperties, deviceFeatures);

    const std::vector<std::string> supportedExtensions = getVkDeviceExtensionNames(physicalDevice);

    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, (deviceProperties.deviceName));

    queueFamilyIndices = indices;
    physicalDeviceProperties = deviceProperties;

    if (!dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->isDeviceSuitable(physicalDevice, deviceProperties, deviceFeatures, supportedExtensions) ||
        !isDeviceSuitable(physicalDevice, deviceProperties, deviceFeatures, supportedExtensions, indices)) return 0;

    uint32_t score = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getDeviceScore(physicalDevice, deviceProperties, deviceFeatures, supportedExtensions);
    logger->logDebug("Found Physical Device '" + std::string(deviceProperties.deviceName) + "' With Score Rating " + std::to_string(score) + "!");

    score += indices.graphicsScore;
    score += indices.gresentScore;

    return score;

}

bool XTPVulkan::isDeviceSuitable([[maybe_unused]] VkPhysicalDevice device,
    [[maybe_unused]] VkPhysicalDeviceProperties properties,
    [[maybe_unused]] VkPhysicalDeviceFeatures features,
    [[maybe_unused]] const std::vector<std::string> &extensions,
    const QueueFamilyIndices &indices) {

    const auto [capabilities, formats, presentModes] = querySwapChainSupport(device);
    const bool adequateSwapchain = !formats.empty() && !presentModes.empty();

    if (!adequateSwapchain) {
        logger->logDebug("Swapchain for device '" + std::string(properties.deviceName) + "' inadequate!");
    }

    return indices.isComplete() && adequateSwapchain;
}

void XTPVulkan::getDeviceInfo(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties &properties,
    VkPhysicalDeviceFeatures &features) {

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    properties = deviceProperties;
    features = deviceFeatures;
}

QueueFamilyIndices XTPVulkan::findQueueFamilies(VkPhysicalDevice device, const std::string &deviceName) {
    QueueFamilyIndices indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    logger->logDebug(std::to_string(queueFamilyCount) + " Queue Families Found!");
    uint32_t i = 0;
    for (VkQueueFamilyProperties queueFamily: queueFamilies) {
        if (dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->isQueueFamilySuitable(queueFamily)) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                const uint32_t graphicsScore = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getQueueFamilyGraphicsScore(queueFamily);
                logger->logDebug("Found Queue Family #" + std::to_string(i) + " With Graphics Score " + std::to_string(graphicsScore) + " For Device '" + deviceName +
                                           "'. This Queue Supports: " + getQueueCaps(queueFamily));

                if (graphicsScore > indices.graphicsScore) {
                    logger->logDebug("Found New Best Graphics Queue Family (#" + std::to_string(graphicsScore) + ") For Device '" + deviceName + "'");
                    indices.graphicsFamily = i;

                    indices.graphicsScore = graphicsScore;
                }
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, getSurface(), &presentSupport);
            if (presentSupport) {
                const uint32_t presentScore = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getQueueFamilyPresentScore(queueFamily);
                logger->logDebug("Found Queue Family #" + std::to_string(i) + " With Present Score " + std::to_string(presentScore) + " For Device '" + deviceName +
                                           "'. This Queue Supports: " + getQueueCaps(queueFamily));
                if (presentScore > indices.gresentScore) {
                    logger->logDebug("Found New Best Present Queue Family (#" + std::to_string(i) + ") For Device '" + deviceName + "'");

                    indices.presentFamily = i;
                    indices.gresentScore = presentScore;
                }
            }
        }

        i++;
    }

    return indices;
}

std::string XTPVulkan::getQueueCaps(const VkQueueFamilyProperties &queueFamily) {
    std::string str;


    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        str += "Graphics, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        str += "Compute, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
        str += "Transfer, ";
    }
    if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        str += "Sparse Memory Management, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
        str += "Video Decode, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
        str += "Video Encode, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
        str += "Optical Flow, ";
    }

    if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
        str += "Protected Memory, ";
    }

    if (!str.empty()) {
        str.pop_back();
        str.pop_back();
    }


    return str;
}

// ReSharper disable once CppNotAllPathsReturnValue
VkInstance XTPVulkan::getVkInstance() {
    if (_vkInstance.has_value()) return _vkInstance.value();
    logger->logError("Attempted To Access A Null Vulkan Instance. Call XTPVulkan::initVulkan() Before Calling This Function.!");
}

VkBool32 XTPVulkan::debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
const VkDebugUtilsMessageTypeFlagsEXT messageType,
const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
void *pUserData) {
    dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->onVulkanDebugMessage(messageSeverity, messageType, pCallbackData, pUserData, std::string(pCallbackData->pMessage));

    return 0;
}

VkResult XTPVulkan::createDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT*pDebugMessenger) {

    if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// ReSharper disable once CppNotAllPathsReturnValue
VkDebugUtilsMessengerEXT XTPVulkan::getDebugMessenger() {
    if (!useValidationLayers) {
        logger->logError("Attempting To Access A Debug Only Function In A Non-Debug Context, Aborting!");
    }

    logger->logDebug("Creating Vulkan Debug Messenger Create Info");
    const VkDebugUtilsMessengerCreateInfoEXT createInfo = getDebugUtilsMessengerCreateInfo();

    if (VkDebugUtilsMessengerEXT messenger; createDebugUtilsMessengerEXT(getVkInstance(), &createInfo, nullptr, &messenger) ==
                                            VK_SUCCESS) return messenger;

    logger->logError("Unable to Initialize Vulkan Debug Messenger!");


}

VkDebugUtilsMessengerCreateInfoEXT XTPVulkan::getDebugUtilsMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
    return info;
}

VkInstance XTPVulkan::createVulkanInstance() {

    if (useValidationLayers && !checkForValidationLayerSupport()) {
        throw std::runtime_error("Could Not Find Requested Validation Layers!");
    }

    logger->logDebug("Creating Vulkan Application Info");
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getApplicationName();
    appInfo.applicationVersion = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getApplicationVersion();
    appInfo.pEngineName = XTP::engineName.data();
    appInfo.engineVersion = XTP::engineVersion;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    logger->logDebug("Creating Vulkan Instance Create Info");
    VkInstanceCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;

    VkInstanceCreateFlags flags {};
    const std::vector<const char*> enabledExtensions = findAllVulkanInstanceExtensions(flags);

    const std::vector enabledLayers = dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getValidationLayers();

    if (useValidationLayers) {
        info.enabledLayerCount = enabledLayers.size();
        info.ppEnabledLayerNames = enabledLayers.data();
    } else {
        info.enabledLayerCount = 0;
    }
    info.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    info.ppEnabledExtensionNames = enabledExtensions.data();
    info.flags = flags;

    logger->logDebug("Creating Vulkan Instance");

    VkInstance instance;
    if (const VkResult result = vkCreateInstance(&info, nullptr, &instance); result != VK_SUCCESS) {
        logger->logCritical("Failed To Create Vulkan Instance!");
        throw std::runtime_error("Unable To Initialize Vulkan");
    }

    logger->logDebug("Created Vulkan Instance With The Following Extensions: " + makeListOf(enabledExtensions));
    if (useValidationLayers) {
        logger->logDebug("Created Vulkan Instance With The Following Validation Layers: " + makeListOf(enabledLayers));
    }

    return instance;
}

void XTPVulkan::printAvailableVulkanProperties() {
    for (auto [extensionName, specVersion]: _availableInstanceExtensions) {
        logger->logDebug("Found Vulkan Instance Extension '" + std::string(extensionName) + "'!");
    }

    for (auto [layerName, specVersion, implementationVersion, description]: _availableValidationLayers) {
        logger->logDebug("Found Vulkan Validation Layer '" + std::string(layerName) + "'!");
    }
}

bool XTPVulkan::checkForValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    _availableValidationLayers = availableLayers;

    return ensureAllValidationLayersPresent(availableLayers);
}

bool XTPVulkan::ensureAllValidationLayersPresent(const std::vector<VkLayerProperties> &availableLayers) {
    bool hasEncounteredMissingLayer = false;
    for (std::string validationLayer: dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getValidationLayers()) {
        bool foundLayer = false;
        for (auto [layerName, specVersion, implementationVersion, description] : availableLayers) {
            if (std::string(layerName) == validationLayer) {
                foundLayer = true;
            }
        }
        if (foundLayer) continue;

        hasEncounteredMissingLayer = true;
        logger->logError("Could Not Find Requested Vulkan Validation Layer '" + validationLayer + "'!");
    }

    return !hasEncounteredMissingLayer;
}

std::string XTPVulkan::getLayerName(const VkLayerProperties &layerProperties) {
    return layerProperties.layerName;
}

std::vector<const char *> XTPVulkan::findAllVulkanInstanceExtensions(VkInstanceCreateFlags &flags) {
    std::vector<const char*> enabledInstanceExtensions;
    VkInstanceCreateFlags createFlags;
    setupAdditionalVulkanInstanceExtensions(enabledInstanceExtensions, createFlags);

    uint32_t extensionCount;
    const char** glfwInstanceExtensions = reinterpret_cast<VulkanWindow*>(XTPWindowing::windowBackend)->getRequiredInstanceExtensions(extensionCount);
    for (int i = 0; i < extensionCount; i++) {
        enabledInstanceExtensions.emplace_back(glfwInstanceExtensions[i]);
    }

    flags = createFlags;

    for (std::string enabledInstanceExtension : enabledInstanceExtensions) {
        bool isPresent = false;
        for (auto [extensionName, specVersion] : _availableInstanceExtensions) {
            if (std::string(extensionName) == enabledInstanceExtension) {
                isPresent = true;
                break;
            }
        }
        if (!isPresent) {
            logger->logCritical("Unable To Find The Required Instance Extension '" + enabledInstanceExtension + "'!");
            throw std::runtime_error("Failed To Find A Required Instance Extension");
        }
    }
    return enabledInstanceExtensions;
}

std::string XTPVulkan::getExtensionName(const VkExtensionProperties &extension) {
    return (extension.extensionName);
}

void XTPVulkan::setupAdditionalVulkanInstanceExtensions(std::vector<const char *> &enabledInstanceExtensions,
    VkInstanceCreateFlags &flags) {
    enabledInstanceExtensions = {};

    // Fixes a Crash Where The Instance Does Not Create Due To Missing Extensions When Using MoltenVK
    for (auto [extensionName, specVersion] : _availableInstanceExtensions) {
        if (std::string(extensionName) == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) {
            enabledInstanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
    }

    for (std::string& instanceExtension: dynamic_cast<VulkanRenderInfo*>(XTPRendering::renderInfo)->getRequiredInstanceExtensions()) {
        enabledInstanceExtensions.emplace_back(instanceExtension.data());
    }

    if (useValidationLayers) {
        enabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

std::vector<VkExtensionProperties> XTPVulkan::getVkInstanceExtensionInfo() {
    uint32_t extensionCount = getVkInstanceExtensionCount();

    logger->logDebug("Found " + std::to_string(extensionCount) + " Vulkan Instance Extensions!");

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

    return extensionProperties;
}

uint32_t XTPVulkan::getVkInstanceExtensionCount() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    return extensionCount;
}

void XTPVulkan::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
    if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")); func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
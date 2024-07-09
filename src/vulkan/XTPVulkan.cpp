#define MVK_CONFIG_SYNCHRONOUS_QUEUE_SUBMITS 0
#define MVK_CONFIG_PRESENT_WITH_COMMAND_BUFFER 1

#define VMA_IMPLEMENTATION
#include "XTPVulkan.h"

#include "FrameEvent.h"
#include "VulkanRenderInfo.h"
#include "VulkanWindow.h"
#include "XTPWindowBackend.h"
#include "XTPWindowing.h"
#include "Event.h"
#include "Events.h"
#include "TimeManager.h"
#include "ShaderRegisterEvent.h"
#include "glm/glm.hpp"
#include <ranges>

VkInstance XTPVulkan::instance;
VkQueue XTPVulkan::presentQueue;
VkQueue XTPVulkan::graphicsQueue;
bool XTPVulkan::useValidationLayers;
VkDebugUtilsMessengerEXT XTPVulkan::debugUtilsMessenger;
std::vector<VkExtensionProperties> XTPVulkan::availableDeviceExtensions;
std::vector<VkLayerProperties> XTPVulkan::availableValidationLayers;
std::vector<VkExtensionProperties> XTPVulkan::availableInstanceExtensions;
VkPhysicalDevice XTPVulkan::gpu;
VkDevice XTPVulkan::device;
QueueFamilyIndices XTPVulkan::queueIndices;
VkSurfaceKHR XTPVulkan::surface;

VkSwapchainKHR XTPVulkan::swapchain;
std::vector<VkImage> XTPVulkan::swapchainImages;
VkFormat XTPVulkan::swapchainImageFormat;
std::vector<VkImageView> XTPVulkan::swapchainImageViews;
VkExtent2D XTPVulkan::swapchainExtent;
std::unordered_map<const char *, VkRenderPass> XTPVulkan::renderPasses;
std::vector<VkFramebuffer> XTPVulkan::swapchainFramebuffers;
std::unordered_map<std::shared_ptr<ShaderObject>, std::vector<std::shared_ptr<Renderable>>> XTPVulkan::toRender;
VkCommandPool XTPVulkan::commandPool;
std::vector<VkCommandBuffer> XTPVulkan::commandBuffers;
std::vector<VkSemaphore> XTPVulkan::imageAvailableSemaphores;
std::vector<VkSemaphore> XTPVulkan::renderFinishedSemaphores;
std::vector<VkFence> XTPVulkan::inFlightFences;
uint32_t XTPVulkan::currentFrameIndex;
bool XTPVulkan::initialized = false;
VmaAllocator XTPVulkan::allocator;

SimpleLogger *XTPVulkan::logger = new SimpleLogger("XTP Builtin Vulkan Renderer", ISDEBUG ? DEBUG : INFORMATION);


bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value() && graphicsScore > 0 && gresentScore > 0;
}

void XTPVulkan::init() {
    if (!XTPWindowing::windowBackend->supportsRenderer("vulkan")) {
        logger->logCritical("Window Backend Must Support Vulkan!");
    }

    logger->logInformation("Initializing Vulkan!");
    useValidationLayers = ISDEBUG;

    logger->logDebug("Getting Vulkan Instance Extension Info");
    availableInstanceExtensions = getVkInstanceExtensionInfo();
    printAvailableVulkanProperties();

    instance = createVulkanInstance();
    if (useValidationLayers) {
        debugUtilsMessenger = getDebugMessenger();
    }

    surface = createSurface();

    QueueFamilyIndices indices;
    gpu = pickPhysicalDevice(indices);
    queueIndices = indices;

    logger->logDebug("Getting Vulkan Device Extension Info");
    availableDeviceExtensions = getVkDeviceExtensionInfo();
    printAvailableDeviceExtensions();

    device = createLogicalDevice();
    logger->logInformation("Finished Initializing Vulkan!");

    swapchain = createSwapchain();

    swapchainImageViews = createImageViews();

    renderPasses.insert({"main", createRenderPass()});

    createFramebuffers();

    commandPool = createCommandPool();
    commandBuffers = createCommandBuffers();
    createSyncObjects();


    Events::callFunctionOnAllEventsOfType<ShaderRegisterEvent>([](ShaderRegisterEvent* event) {event->onRegisterShaders();});

    initialized = true;

    allocator = createAllocator();
}

void XTPVulkan::recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = getMainRenderPass();
    renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent;

    const VkClearValue clearColor = VulkanRenderInfo::INSTANCE->getClearColor();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ShaderObject* currentShader = nullptr;
    auto pairIterator = toRender.begin();
    while(pairIterator != toRender.end()) {
        bool isFirstShader = false;
        if (currentShader == nullptr) {
            currentShader = pairIterator->first.get();
            isFirstShader = true;
        }
        if (pairIterator->first.get() != currentShader || isFirstShader) {
            currentShader = pairIterator->first.get();
            currentShader->prepareForRender(commandBuffer);
        }

        auto renderableIterator = pairIterator->second.begin();
        while (renderableIterator != pairIterator->second.end()) {
            bool removed = false;
            if(renderableIterator->get()->shouldRemove()) {
                renderableIterator->get()->remove();
                renderableIterator = pairIterator->second.erase(renderableIterator);
                removed = true;
            } else if (!renderableIterator->get()->hasInitialized()) {
                renderableIterator->get()->init();
            }
            renderableIterator->get()->draw(commandBuffer, imageIndex);

            if (!removed) ++renderableIterator;
        }
        ++pairIterator;
    }

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void XTPVulkan::drawFrame() {
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent* event) {event->onFrameBegin();});
    if (XTPWindowing::windowBackend->framebufferResized) {
        XTPWindowing::windowBackend->framebufferResized = false;
        recreateSwapchain();
    }

    vkWaitForFences(device, 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);

    vkResetFences(device, 1, &inFlightFences[currentFrameIndex]);

    vkResetCommandBuffer(commandBuffers[currentFrameIndex],  0);
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent* event) {event->onFrameDrawBegin();});
    recordCommandBuffers(commandBuffers[currentFrameIndex], imageIndex);
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent* event) {event->onFrameDrawEnd();});

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrameIndex]};
    constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrameIndex];

    const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrameIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrameIndex]) != VK_SUCCESS) {
        logger->logCritical("Failed To Submit Draw Command Buffer!");
    }

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {swapchain};

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    currentFrameIndex = (currentFrameIndex + 1) % VulkanRenderInfo::INSTANCE->getMaxFramesInFlight();
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent* event) {event->onFrameEnd();});
}

void XTPVulkan::cleanupSwapchain() {
    for (const auto & swapchainFramebuffer : swapchainFramebuffers) {
        vkDestroyFramebuffer(device, swapchainFramebuffer, nullptr);
    }

    for (const auto & swapchainImageView : swapchainImageViews) {
        vkDestroyImageView(device, swapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void XTPVulkan::recreateSwapchain() {
    vkDeviceWaitIdle(device);
    int width = 0, height = 0;
    XTPWindowing::getFramebufferSize(&width, &height);
    while (width == 0 || height == 0) {
        if (XTPWindowing::windowBackend->shouldClose()) {
            break;
        }
        XTPWindowing::getFramebufferSize(&width, &height);
        XTPWindowing::windowBackend->waitForEvents();
    }


    cleanupSwapchain();

    swapchain = createSwapchain();
    swapchainImageViews = createImageViews();
    createFramebuffers();
}


std::vector<VkCommandBuffer> XTPVulkan::createCommandBuffers() {
    std::vector<VkCommandBuffer> commandBuffers(VulkanRenderInfo::INSTANCE->getMaxFramesInFlight());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        logger->logCritical("Failed To Allocate Command Buffers!");
    }

    return commandBuffers;
}


VkCommandPool XTPVulkan::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(gpu);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkCommandPool commandPool;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        logger->logCritical("Failed To Create Command Buffer!");
    }

    return commandPool;
}

void XTPVulkan::createSyncObjects() {
    imageAvailableSemaphores.resize(VulkanRenderInfo::INSTANCE->getMaxFramesInFlight());
    renderFinishedSemaphores.resize(VulkanRenderInfo::INSTANCE->getMaxFramesInFlight());
    inFlightFences.resize(VulkanRenderInfo::INSTANCE->getMaxFramesInFlight());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(); i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            logger->logCritical("Failed To Create Sephamores!");
        }
    }
}

VmaAllocator XTPVulkan::createAllocator() {
    VmaAllocator allocator;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gpu;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    return allocator;
}


void XTPVulkan::cleanUp() {
    vkDeviceWaitIdle(device);
    logger->logDebug("Cleaning Up Vulkan");


    for (auto& [shader, renderables]: toRender) {
        shader->cleanUp();
        for (const std::shared_ptr<Renderable>& renderable : renderables) {
            renderable->remove();
        }
    }
    toRender.clear();

    for (const std::pair<const char *, VkRenderPass> renderPass: renderPasses) {
        vkDestroyRenderPass(device, renderPass.second, nullptr);
    }

    vmaDestroyAllocator(allocator);

    for (size_t i = 0; i < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(); i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    cleanupSwapchain();

    logger->logDebug("Destroying Vulkan Surface");
    vkDestroySurfaceKHR(instance, surface, nullptr);
    logger->logDebug("Destroying Vulkan Device");
    vkDestroyDevice(device, nullptr);

    if (useValidationLayers) {
        logger->logDebug("Destroying Debug Utils Messenger");
        destroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
    }

    logger->logDebug("Destroying Vulkan Instance");
    vkDestroyInstance(instance, nullptr);
    logger->logDebug("Destroying Vulkan");
}

std::string XTPVulkan::makeListOf(const std::vector<const char *> &elements) {
    std::string str;
    int i = 0;
    for (const char *element: elements) {
        i++;
        str.append(element);
        if (i != elements.size()) {
            str.append(", ");
        }
    }
    return str;
}

void XTPVulkan::addShader(const std::shared_ptr<ShaderObject>& shader) {
    shader->init();
    toRender.insert({shader, {}});
    int i = 0;
}

void XTPVulkan::addRenderable(const std::shared_ptr<Renderable>& renderable) {
    toRender.at(renderable->getShader()).emplace_back(renderable);
    int i = 0;
}

std::vector<VkImageView> XTPVulkan::createImageViews() {
    std::vector<VkImageView> views(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &views[i]) != VK_SUCCESS) {
            logger->logCritical("Failed To Create Image Views!!");
        }
    }
    return views;
}

void XTPVulkan::printAvailableDeviceExtensions() {
    logger->logDebug("Found " + std::to_string(availableDeviceExtensions.size()) + " Vulkan Device Extensions!");
    for (VkExtensionProperties properties: availableDeviceExtensions) {
        logger->logDebug("Found Vulkan Device Extension '" + std::string(properties.extensionName) + "'!");
    }
}

// ReSharper disable once CppNotAllPathsReturnValue
VkSurfaceKHR XTPVulkan::createSurface() {
    VkSurfaceKHR surface;
    logger->logDebug("Creating Vulkan Surface");
    if (dynamic_cast<VulkanWindow*>(XTPWindowing::windowBackend.get())->createVulkanSurface(instance, &surface)) return
            surface;

    logger->logCritical("Unable To Create Vulkan Surface");
}

VkDevice XTPVulkan::createLogicalDevice() {
    logger->logDebug("Creating Vulkan Device Queue Create Info");
    float queuePriority = 1;
    const std::set uniqueQueueFamilies = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> infos(uniqueQueueFamilies.size());

    uint32_t i = 0;
    for (const uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = static_cast<uint32_t>(queueFamily);
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;

        infos[i] = info;
        i++;
    }


    logger->logDebug("Creating Vulkan Device Create Info");
    std::vector<const char *> layers;
    std::vector<const char *> deviceExtensions;

    std::vector<const char *> enabledDeviceExtensions = {};

    for (const auto [extensionName, specVersion]: availableDeviceExtensions) {
        if (strcmp(extensionName, "VK_KHR_portability_subset") == 0) {
            enabledDeviceExtensions = {"VK_KHR_portability_subset"};
        }
    }

    enabledDeviceExtensions.emplace_back("VK_KHR_swapchain");

    for (std::string extension: VulkanRenderInfo::INSTANCE->getRequiredDeviceExtensions()) {
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


    const VkPhysicalDeviceFeatures features = VulkanRenderInfo::INSTANCE->getPhysicalDeviceFeatures();

    VkDeviceCreateInfo deviceCreateInfo{};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = infos.data();
    deviceCreateInfo.queueCreateInfoCount = infos.size();
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensions.size();

    // For Compatibility With Older Vulkan Implementations That Distinguished Between Instance And Device Validation Layers
    const std::vector<const char *> enabledLayers = VulkanRenderInfo::INSTANCE->getValidationLayers();

    if (useValidationLayers) {
        deviceCreateInfo.enabledLayerCount = enabledLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }


    VkDevice device;
    logger->logDebug("Creating Vulkan Device");
    if (const VkResult result = vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device); result != VK_SUCCESS) {
        logger->logCritical("Unable To Create Vulkan Logical Device");
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, queueIndices.graphicsFamily.value_or(0), 0, &graphicsQueue);
    XTPVulkan::graphicsQueue = graphicsQueue;


    VkQueue presentQueue;
    vkGetDeviceQueue(device, queueIndices.presentFamily.value_or(0), 0, &presentQueue);
    XTPVulkan::presentQueue = presentQueue;

    return device;
}

VkRenderPass XTPVulkan::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;

    //todo: work with multisampling
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;


    VkRenderPass renderPass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        logger->logCritical("Failed To Create Render Pass!");
    }

    return renderPass;
}

VkSwapchainKHR XTPVulkan::createSwapchain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(gpu);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = findQueueFamilies(gpu);

    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        logger->logCritical("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    return swapchain;
}

void XTPVulkan::createFramebuffers() {
    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        const VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = getMainRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            logger->logCritical("Failed To Create Framebuffer!");
        }
    }
}

VkRenderPass XTPVulkan::getMainRenderPass() {
    return renderPasses.at("main");
}

void XTPVulkan::render() {
    XTPWindowing::windowBackend->beginFrame();
    drawFrame();
    TimeManager::endFrame();
}

bool XTPVulkan::hasInitialized() {
    return initialized;
}

VkPresentModeKHR XTPVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const VkPresentModeKHR &presentMode: availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D XTPVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    XTPWindowing::getFramebufferSize(&width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}

SwapChainSupportDetails XTPVulkan::querySwapChainSupport(const VkPhysicalDevice &device) {
    SwapChainSupportDetails details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (formatCount > 0) {
        details.presentModes.resize(formatCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR XTPVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    logger->logDebug("Choosing Swap Surface Format!");
    for (const VkSurfaceFormatKHR availableFormat: availableFormats) {
        logger->logDebug("Found Surface Format " + VkFormatParser::toStringVkFormat(availableFormat.format));
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

std::vector<VkExtensionProperties> XTPVulkan::getVkDeviceExtensionInfo() {
    VkPhysicalDevice device = gpu;

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> properties(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, properties.data());

    return properties;
}

std::vector<std::string> XTPVulkan::getVkDeviceExtensionNames(VkPhysicalDevice device) {
    if (device == nullptr) {
        device = gpu;
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
        logger->logDebug(
            "Found New Best Physical Device '" + std::string(deviceProperties.deviceName) + "', Using This Device!");
    }

    if (foundSuitablePhysicalDevice && deviceScore != 0 && device.has_value()) return device.value();

    logger->logCritical("Unable To Find A Suitable GPU!");
    throw std::runtime_error("No Suitable GPU Was Found!");
}

std::vector<VkPhysicalDevice> XTPVulkan::getAllPhysicalDevices() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        logger->logCritical("Unable To Find A GPU With Vulkan Support!");
        throw std::runtime_error("No GPU With Vulkan Support Was Found!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    return devices;
}

uint32_t XTPVulkan::getDeviceScore(VkPhysicalDevice physicalDevice, QueueFamilyIndices &queueFamilyIndices,
                                   VkPhysicalDeviceProperties &physicalDeviceProperties) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    getDeviceInfo(physicalDevice, deviceProperties, deviceFeatures);

    const std::vector<std::string> supportedExtensions = getVkDeviceExtensionNames(physicalDevice);

    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, (deviceProperties.deviceName), true);

    queueFamilyIndices = indices;
    physicalDeviceProperties = deviceProperties;

    if (!VulkanRenderInfo::INSTANCE->isDeviceSuitable(physicalDevice, deviceProperties, deviceFeatures,
                                                      supportedExtensions) ||
        !isDeviceSuitable(physicalDevice, deviceProperties, deviceFeatures, supportedExtensions, indices))
        return 0;

    uint32_t score = VulkanRenderInfo::INSTANCE->getDeviceScore(physicalDevice, deviceProperties, deviceFeatures,
                                                                supportedExtensions);
    logger->logDebug(
        "Found Physical Device '" + std::string(deviceProperties.deviceName) + "' With Score Rating " +
        std::to_string(score) + "!");

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

QueueFamilyIndices
XTPVulkan::findQueueFamilies(VkPhysicalDevice device, const std::string &deviceName, bool printDebug) {
    QueueFamilyIndices indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    if (printDebug) logger->logDebug(std::to_string(queueFamilyCount) + " Queue Families Found!");
    uint32_t i = 0;
    for (VkQueueFamilyProperties queueFamily: queueFamilies) {
        if (VulkanRenderInfo::INSTANCE->isQueueFamilySuitable(queueFamily)) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                const uint32_t graphicsScore = VulkanRenderInfo::INSTANCE->getQueueFamilyGraphicsScore(queueFamily);
                if (printDebug)
                    logger->logDebug(
                        "Found Queue Family #" + std::to_string(i) + " With Graphics Score " +
                        std::to_string(graphicsScore) + " For Device '" + deviceName +
                        "'. This Queue Supports: " + getQueueCaps(queueFamily));

                if (graphicsScore > indices.graphicsScore) {
                    if (printDebug) logger->logDebug(
                        "Found New Best Graphics Queue Family (#" + std::to_string(graphicsScore) + ") For Device '" +
                        deviceName + "'");
                    indices.graphicsFamily = i;

                    indices.graphicsScore = graphicsScore;
                }
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                const uint32_t presentScore = VulkanRenderInfo::INSTANCE->getQueueFamilyPresentScore(queueFamily);
                if (printDebug)
                    logger->logDebug(
                        "Found Queue Family #" + std::to_string(i) + " With Present Score " +
                        std::to_string(presentScore) + " For Device '" + deviceName +
                        "'. This Queue Supports: " + getQueueCaps(queueFamily));
                if (presentScore > indices.gresentScore) {
                    if (printDebug) logger->logDebug(
                        "Found New Best Present Queue Family (#" + std::to_string(i) + ") For Device '" + deviceName +
                        "'");

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

VkBool32 XTPVulkan::debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  const VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *pUserData) {
    VulkanRenderInfo::INSTANCE->onVulkanDebugMessage(messageSeverity, messageType, pCallbackData, pUserData,
                                                     std::string(pCallbackData->pMessage));

    return 0;
}

VkResult XTPVulkan::createDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger) {
    if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
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

    if (VkDebugUtilsMessengerEXT messenger; createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &messenger) ==
                                            VK_SUCCESS)
        return messenger;

    logger->logError("Unable to Initialize Vulkan Debug Messenger!");
}

VkDebugUtilsMessengerCreateInfoEXT XTPVulkan::getDebugUtilsMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
    return info;
}

VkInstance XTPVulkan::createVulkanInstance() {
    if (useValidationLayers && !checkForValidationLayerSupport()) {
        throw std::runtime_error("Could Not Find Requested Validation Layers!");
    }

    logger->logDebug("Creating Vulkan Application Info");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = VulkanRenderInfo::INSTANCE->getApplicationName();
    appInfo.applicationVersion = VulkanRenderInfo::INSTANCE->getApplicationVersion();
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    logger->logDebug("Creating Vulkan Instance Create Info");
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;

    VkInstanceCreateFlags flags{};
    const std::vector<const char *> enabledExtensions = findAllVulkanInstanceExtensions(flags);

    const std::vector enabledLayers = VulkanRenderInfo::INSTANCE->getValidationLayers();

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
    for (auto [extensionName, specVersion]: availableInstanceExtensions) {
        logger->logDebug("Found Vulkan Instance Extension '" + std::string(extensionName) + "'!");
    }

    for (auto [layerName, specVersion, implementationVersion, description]: availableValidationLayers) {
        logger->logDebug("Found Vulkan Validation Layer '" + std::string(layerName) + "'!");
    }
}

bool XTPVulkan::checkForValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    availableValidationLayers = availableLayers;

    return ensureAllValidationLayersPresent(availableLayers);
}

bool XTPVulkan::ensureAllValidationLayersPresent(const std::vector<VkLayerProperties> &availableLayers) {
    bool hasEncounteredMissingLayer = false;
    for (std::string validationLayer: VulkanRenderInfo::INSTANCE->getValidationLayers()) {
        bool foundLayer = false;
        for (auto [layerName, specVersion, implementationVersion, description]: availableLayers) {
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
    std::vector<const char *> enabledInstanceExtensions;
    VkInstanceCreateFlags createFlags;
    setupAdditionalVulkanInstanceExtensions(enabledInstanceExtensions, createFlags);

    uint32_t extensionCount;
    const char **glfwInstanceExtensions = dynamic_cast<VulkanWindow*>(XTPWindowing::windowBackend.get())->
            getRequiredInstanceExtensions(extensionCount);
    for (int i = 0; i < extensionCount; i++) {
        enabledInstanceExtensions.emplace_back(glfwInstanceExtensions[i]);
    }

    flags = createFlags;

    for (std::string enabledInstanceExtension: enabledInstanceExtensions) {
        bool isPresent = false;
        for (auto [extensionName, specVersion]: availableInstanceExtensions) {
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
    for (auto [extensionName, specVersion]: availableInstanceExtensions) {
        if (std::string(extensionName) == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) {
            enabledInstanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
    }

    for (std::string &instanceExtension: VulkanRenderInfo::INSTANCE->getRequiredInstanceExtensions()) {
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

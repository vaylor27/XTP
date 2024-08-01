#define MVK_CONFIG_SYNCHRONOUS_QUEUE_SUBMITS 1
// #define MVK_CONFIG_PRESENT_WITH_COMMAND_BUFFER 1

#define VMA_IMPLEMENTATION
#define VMA_BUFFER_DEVICE_ADDRESS true
#include "XTPVulkan.h"
#ifdef XTP_USE_IMGUI_UI
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#endif

#include "FrameEvent.h"
#include "VulkanRenderInfo.h"
#include "XTPWindowBackend.h"
#include "XTPWindowing.h"
#include "Events.h"
#include "TimeManager.h"
#include "ShaderRegisterEvent.h"
#include "glm/glm.hpp"
#include <ranges>
#include "AllocatedImage.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "tiny_gltf.h"

#include "Camera.h"

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#include "TracyClient.cpp"
#else
#define TracyCZoneN(c, e, t)
#define TracyCZoneEnd(c)
#define FrameMark
#endif

#include "RenderDebugUIEvent.h"
#include "VkFormatParser.h"

VkInstance XTPVulkan::instance;
VkQueue XTPVulkan::presentQueue;
VkQueue XTPVulkan::graphicsQueue;
#ifdef ISDEBUG
VkDebugUtilsMessengerEXT XTPVulkan::debugUtilsMessenger;
#endif
#ifdef XTP_USE_IMGUI_UI
VkDescriptorPool XTPVulkan::imguiPool;
#endif
#ifdef XTP_USE_ADVANCED_TIMING
VkQueryPool XTPVulkan::timeQueryPool;
std::vector<uint64_t> XTPVulkan::renderTimeNanos;
std::vector<bool> XTPVulkan::timeQueryInitialized;
#endif

#ifdef XTP_USE_GLTF_LOADING
tinygltf::TinyGLTF XTPVulkan::gltfLoader {};
#endif
std::vector<VkExtensionProperties> XTPVulkan::availableDeviceExtensions;
std::vector<VkLayerProperties> XTPVulkan::availableValidationLayers;
std::vector<VkExtensionProperties> XTPVulkan::availableInstanceExtensions;
VkPhysicalDevice XTPVulkan::gpu;
VkDevice XTPVulkan::device;
QueueFamilyIndices XTPVulkan::queueIndices;
VkSurfaceKHR XTPVulkan::surface;
std::vector<bool> XTPVulkan::i;
std::vector<bool> XTPVulkan::doesSceneBufferNeedToBeUpdated;
AllocatedImage XTPVulkan::depthImage;
uint32_t XTPVulkan::mostRecentFrameRendered;
VkSwapchainKHR XTPVulkan::swapchain;
std::vector<VkImage> XTPVulkan::swapchainImages;
VkFormat XTPVulkan::swapchainImageFormat;
std::vector<VkImageView> XTPVulkan::swapchainImageViews;
VkExtent2D XTPVulkan::swapchainExtent;
std::vector<VkFramebuffer> XTPVulkan::swapchainFramebuffers;
std::unordered_map<std::shared_ptr<ShaderObject>, std::unordered_map<std::shared_ptr<Material>, std::vector<
    std::shared_ptr<Renderable> > > > XTPVulkan::toRender;
VkCommandPool XTPVulkan::commandPool;
std::vector<VkCommandBuffer> XTPVulkan::commandBuffers;
std::vector<VkSemaphore> XTPVulkan::imageAvailableSemaphores;
std::vector<VkSemaphore> XTPVulkan::renderFinishedSemaphores;
std::vector<VkFence> XTPVulkan::inFlightFences;
std::unique_ptr<vke::DescriptorAllocatorPool> XTPVulkan::allocatorPool;
uint32_t XTPVulkan::currentFrameIndex;
bool XTPVulkan::initialized = false;
VmaAllocator XTPVulkan::allocator;
VkPhysicalDeviceProperties XTPVulkan::gpuProperties;
std::vector<AllocatedBuffer> XTPVulkan::buffers;
AllocatedImage XTPVulkan::errorTexure;
std::vector<AllocatedImage> XTPVulkan::allLoadedImages;
std::vector<VkSampler> XTPVulkan::samplers;
glm::mat4 XTPVulkan::projectionMatrix;
glm::mat4 XTPVulkan::viewMatrix;
std::vector<AllocatedBuffer> XTPVulkan::globalSceneDataBuffers;
bool XTPVulkan::shouldUpdateProjectionMatrix = true;
bool XTPVulkan::initializedErrorTex = false;
VkRenderPass XTPVulkan::renderPass;
#ifdef ISDEBUG
SimpleLogger *XTPVulkan::logger = new SimpleLogger("Debug Vulkan Renderer", DEBUG);
#else
SimpleLogger *XTPVulkan::logger = new SimpleLogger("Vulkan Renderer", INFORMATION);
#endif

void XTPVulkan::init() {
    ZoneScopedN("XTPVulkan::init");

    logger->logInformation("Initializing Vulkan!");

    logger->logDebug("Getting Vulkan Instance Extension Info");
    availableInstanceExtensions = getVkInstanceExtensionInfo();
    printAvailableVulkanProperties();

    instance = createVulkanInstance();
#ifdef  ISDEBUG
    debugUtilsMessenger = getDebugMessenger();
#endif

    surface = createSurface();

    QueueFamilyIndices indices;
    gpu = pickPhysicalDevice(indices);
    queueIndices = indices;

    logger->logDebug("Getting Vulkan Device Extension Info");
    availableDeviceExtensions = getVkDeviceExtensionInfo();
    printAvailableDeviceExtensions();

    device = createLogicalDevice();

    swapchain = createSwapchain();

    swapchainImageViews = createImageViews();

    renderPass = createRenderPass();

    allocator = createAllocator();

    depthImage = createDepthImage();
    createFramebuffers();

    commandPool = createCommandPool();
    commandBuffers = createCommandBuffers();

    createSyncObjects();

    initialized = true;

    vkGetPhysicalDeviceProperties(gpu, &gpuProperties);

    uint32_t maxffs = VulkanRenderInfo::INSTANCE->getMaxFramesInFlight();

    i = std::vector<bool>(maxffs);
    doesSceneBufferNeedToBeUpdated = std::vector<bool>(maxffs);
    globalSceneDataBuffers = std::vector<AllocatedBuffer>(maxffs);
    for (int i = 0; i < maxffs; ++i) {
        doesSceneBufferNeedToBeUpdated[i] = true;
        globalSceneDataBuffers[i] = createSimpleBuffer(sizeof(SceneRenderData),
                                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                       VMA_MEMORY_USAGE_CPU_COPY, true);
    }

#ifdef XTP_USE_IMGUI_UI
    imguiPool = initImgui();
#endif
#ifdef XTP_USE_ADVANCED_TIMING
    timeQueryPool = createTimeQueryPool();
    timeQueryInitialized = std::vector<bool>(maxffs);
    renderTimeNanos = std::vector<uint64_t>(maxffs);
    for (int i = 0; i < maxffs; ++i) {
        renderTimeNanos[i] = 0;
    }
#endif
    logger->logInformation("Finished Initializing Vulkan!");

    allocatorPool = std::unique_ptr<vke::DescriptorAllocatorPool>(vke::DescriptorAllocatorPool::Create(device, 1));
    errorTexure = createErrorTex();
    initializedErrorTex = true;

    Events::callFunctionOnAllEventsOfType<ShaderRegisterEvent>([](ShaderRegisterEvent *event) {
        event->onRegisterShaders();
    });
}

glm::mat4 XTPVulkan::calculateProjectionMatrix(const float fov, const float zNear, const float zFar) {
    ZoneScopedN("XTPVulkan::calculateProjectionMatrix");
    const float aspectRatio = static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height);
    const float yScale = 1 / tan(glm::radians(fov / 2)) * aspectRatio;
    const float xScale = yScale / aspectRatio;
    const float frustumLength = zFar - zNear;

    auto projectionMatrix = glm::identity<glm::mat4>();

    projectionMatrix[0][0] = xScale;
    projectionMatrix[1][1] = yScale;
    projectionMatrix[2][2] = -((zFar + zNear) / frustumLength);
    projectionMatrix[2][3] = -1;
    projectionMatrix[3][2] = -(2 * zNear * zFar / frustumLength);
    projectionMatrix[3][3] = 0;

    // projectionMatrix[1][1] *= -1;

    return projectionMatrix;
}

void XTPVulkan::recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex) {
    ZoneScopedN("XTPVulkan::recordCommandBuffers");
#ifdef XTP_USE_ADVANCED_TIMING
    renderTimeNanos[frameIndex - 1 == -1 ? VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(): frameIndex - 1] = fetchRenderTimeNanos();
#endif


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

#ifdef XTP_USE_ADVANCED_TIMING
    // Queries must be reset after each individual use.
    vkResetQueryPool(device, timeQueryPool, frameIndex * 2, 2);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timeQueryPool, frameIndex * 2);
#endif

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = VulkanRenderInfo::INSTANCE->getClearColor();
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ShaderObject* currentShader = nullptr;
    auto pairIterator = toRender.begin();
    while (pairIterator != toRender.end()) {
        bool isFirstShader = false;
        if (currentShader == nullptr) {
            currentShader = pairIterator->first.get();
            isFirstShader = true;
        }
        if (pairIterator->first.get() != currentShader || isFirstShader) {
            currentShader = pairIterator->first.get();
            currentShader->prepareForRender(commandBuffer);
            isFirstShader = false;
        }
        auto materialIterator = pairIterator->second.begin();
        while (materialIterator != pairIterator->second.end()) {
            if (!materialIterator->first->initialized()) {
                materialIterator->first->init(currentShader);
            }
            materialIterator->first->prepareForRender(commandBuffer, currentShader);
            auto renderableIterator = materialIterator->second.begin();
            while (renderableIterator != materialIterator->second.end()) {
                bool removed = false;
                if (renderableIterator->get()->shouldRemove()) {
                    renderableIterator->get()->remove();
                    renderableIterator = materialIterator->second.erase(renderableIterator);
                    removed = true;
                } else if (!renderableIterator->get()->hasInitialized()) {
                    if (!renderableIterator->get()->getMesh()->initialized()) {
                        renderableIterator->get()->getMesh()->init();
                    }
                    pairIterator->first->initRenderable(renderableIterator->get());
                    renderableIterator->get()->init();
                }
                renderableIterator->get()->draw(commandBuffer, imageIndex);
                if (!removed) ++renderableIterator;
            }
            ++materialIterator;
        }
        ++pairIterator;
    }

#ifdef XTP_USE_IMGUI_UI
    ImGui_ImplVulkan_NewFrame();
    XTPWindowing::windowBackend->endFrame();
    ImGui::NewFrame();

    Events::callFunctionOnAllEventsOfType<RenderDebugUIEvent>([](auto event) {event->renderDebugUI();});

    // make imgui calculate internal draw structures
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
#endif

    vkCmdEndRenderPass(commandBuffer);

#ifdef XTP_USE_ADVANCED_TIMING
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timeQueryPool, frameIndex * 2 + 1);
    timeQueryInitialized[frameIndex] = true;
#endif
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void XTPVulkan::drawFrame() {
    ZoneScopedN("XTPVulkan::drawFrame");
    mostRecentFrameRendered = currentFrameIndex;
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent *event) { event->onFrameBegin(); });
    if (XTPWindowing::windowBackend->framebufferResized) {
        XTPWindowing::windowBackend->framebufferResized = false;
        recreateSwapchain();
        shouldUpdateProjectionMatrix = true;
    }

    TracyCZoneN(__waitForFences, "XTPVulkan::drawFrame#waitForFences", true)
    vkWaitForFences(device, 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);
    TracyCZoneEnd(__waitForFences)
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE,
                          &imageIndex);

    if (doesSceneBufferNeedToBeUpdated[currentFrameIndex]) {
        SceneRenderData data = {};
        data.projectionMatrix = projectionMatrix;
        data.viewMatrix = viewMatrix;

        memcpy(globalSceneDataBuffers[currentFrameIndex].info.pMappedData, &data, sizeof(SceneRenderData));
    }

    TracyCZoneN(__reset, "XTPVulkan::drawFrame#reset", true)
    vkResetFences(device, 1, &inFlightFences[currentFrameIndex]);
    vkResetCommandBuffer(commandBuffers[currentFrameIndex], 0);
    TracyCZoneEnd(__reset)

    TracyCZoneN(__draw, "XTPVulkan::drawFrame#draw", true)
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent *event) { event->onFrameDrawBegin(); });
    recordCommandBuffers(commandBuffers[currentFrameIndex], imageIndex, currentFrameIndex);
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent *event) { event->onFrameDrawEnd(); });
    TracyCZoneEnd(__draw)

    TracyCZoneN(subInfCreate, "XTPVulkan::drawFrame#createSubmitInfo", true)
    VkSubmitInfo submitInfo{};
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
    TracyCZoneEnd(subInfCreate)

    TracyCZoneN(submit, "XTPVulkan::drawFrame#submit", true)
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrameIndex]) != VK_SUCCESS) {
        logger->logCritical("Failed To Submit Draw Command Buffer!");
    }
    TracyCZoneEnd(submit)

    TracyCZoneN(presInfCreate, "XTPVulkan::drawFrame#createPresentInfo", true)
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    TracyCZoneEnd(presInfCreate)

    TracyCZoneN(present, "XTPVulkan::drawFrame#present", true);
    vkQueuePresentKHR(presentQueue, &presentInfo);
    TracyCZoneEnd(present)

    i[currentFrameIndex] = true;
    Events::callFunctionOnAllEventsOfType<FrameEvent>([](FrameEvent *event) { event->onFrameEnd(); });
}

void XTPVulkan::cleanupSwapchain() {
    ZoneScopedN("XTPVulkan::cleanupSwapchain");
    for (const auto &swapchainFramebuffer: swapchainFramebuffers) {
        vkDestroyFramebuffer(device, swapchainFramebuffer, nullptr);
    }

    for (const auto &swapchainImageView: swapchainImageViews) {
        vkDestroyImageView(device, swapchainImageView, nullptr);
    }

    destroyAllocatedImage(&depthImage);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void XTPVulkan::recreateSwapchain() {
    ZoneScopedN("XTPVulkan::recreateSwapchain");
    vkDeviceWaitIdle(device);
    int width = 0, height = 0;
    XTPWindowing::windowBackend->getFramebufferSize(&width, &height);
    while (width == 0 || height == 0) {
        if (XTPWindowing::windowBackend->shouldClose()) {
            break;
        }
        XTPWindowing::windowBackend->getFramebufferSize(&width, &height);
        XTPWindowing::windowBackend->waitForEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapchain();

    swapchain = createSwapchain();
    swapchainImageViews = createImageViews();
    depthImage = createDepthImage();
    createFramebuffers();
}

std::vector<VkCommandBuffer> XTPVulkan::createCommandBuffers() {
    ZoneScopedN("XTPVulkan::createCommandBuffers");
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
    ZoneScopedN("XTPVulkan::createCommandPool");
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
    ZoneScopedN("XTPVulkan::createSyncObjects");
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
    ZoneScopedN("XTPVulkan::createAllocator");
    VmaAllocator allocator;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gpu;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    return allocator;
}

VkDescriptorPool XTPVulkan::initImgui() {
#ifdef XTP_USE_IMGUI_UI
    ZoneScopedN("XTPVulkan::initImgui");
    	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	if(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
	    logger->logCritical("Unable to Create imgui descriptor pool");
	}

	// this initializes the core structures of imgui
	ImGui::CreateContext();

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = gpu;
	init_info.Device = device;
	init_info.Queue = graphicsQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = swapchainImages.size();
	init_info.UseDynamicRendering = false;
    init_info.RenderPass = renderPass;

	//dynamic rendering parameters for imgui to use
	init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;

	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info);

    return imguiPool;
#else
    return nullptr;
#endif
}

VkQueryPool XTPVulkan::createTimeQueryPool() {
    ZoneScopedN("XTPVulkan::createTimeQueryPool");
#ifdef XTP_USE_ADVANCED_TIMING
    VkQueryPoolCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = 2 * VulkanRenderInfo::INSTANCE->getMaxFramesInFlight();
    VkQueryPool timeQueryPool;
    if (vkCreateQueryPool(device, &createInfo, nullptr, &timeQueryPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create query pool!!");
    }

    return timeQueryPool;
#else
    return nullptr;
#endif
}

void XTPVulkan::DescriptorSet::setData(const AllocatedBuffer &buffer, uint32_t binding, uint32_t arrayElement,
    uint32_t offset, uint32_t size) const {
    VkDescriptorBufferInfo bufferInfo {};
    bufferInfo.buffer = buffer.internalBuffer;
    bufferInfo.offset = offset;
    bufferInfo.range = size == -1 ? buffer.info.size: size;

    VkWriteDescriptorSet descriptorWrite {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = arrayElement;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void XTPVulkan::DescriptorSet::setData(AllocatedImage image, uint32_t binding, uint32_t arrayElement) const {
    VkDescriptorImageInfo imageInfo {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image.imageView;
    imageInfo.sampler = image.sampler;

    VkWriteDescriptorSet descriptorWrite {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = arrayElement;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void XTPVulkan::DescriptorSet::bind(VkCommandBuffer commandBuffer, ShaderObject *shader) const {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getLayout(), 0, 1, &set, 0, nullptr);
}

XTPVulkan::DescriptorSet XTPVulkan::createDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool,
    ShaderObject *shader, const VkDescriptorType type) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
        logger->logCritical("Failed To Create Descriptor Set!");
    }

    return {set, type};
}

void XTPVulkan::loadGltfModel(bool isBinaryGltf, const char *path, const char *sceneToLoad) {
    tinygltf::Model model;
    std::string err;
    std::string warn;
    (isBinaryGltf ? gltfLoader.LoadASCIIFromFile : gltfLoader.LoadBinaryFromFile)(&model, &err, &warn, path, tinygltf::REQUIRE_VERSION);

    tinygltf::Scene scene;
    if (sceneToLoad == nullptr) {
        scene = model.scenes[0];
    } else {
        for (const auto& scn : model.scenes) {
            if (strcmp(sceneToLoad, scene.name.c_str()) == 0) {
                scene = scn;
                break;
            }
        }
    }

    std::vector<tinygltf::Node> nodes(scene.nodes.size());

    for (int i = 0; i < scene.nodes.size(); ++i) {
        nodes[i] = model.nodes[scene.nodes[i]];
    }
}

uint64_t XTPVulkan::fetchRenderTimeNanos() {
#ifdef XTP_USE_ADVANCED_TIMING
    std::vector<long double> times;

    for (int frame = 0; frame < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(); ++frame) {
        if (timeQueryInitialized[frame]) {
            uint64_t buffer[2];
            VkResult result = vkGetQueryPoolResults(device, timeQueryPool, frame * 2, 2, sizeof(uint64_t) * 2, buffer, sizeof(uint64_t),
                                                    VK_QUERY_RESULT_64_BIT);
            if (result == VK_NOT_READY)
            {
                continue;
            }
            if (result == VK_SUCCESS)
            {
                const uint64_t time = buffer[1] - buffer[0];
                const long double tsp = gpuProperties.limits.timestampPeriod;
                auto nanoTime = time * tsp;
                times.emplace_back(nanoTime);
            }
            else
            {
                throw std::runtime_error("Failed to receive query results!");
            }
        }
        if (times.empty()) {
            return -1;
        }
        uint64_t sum = 0;
        for (const long double time: times) {
            sum += static_cast<uint64_t>(time);
        }
        return sum / times.size();
    }
    return -1;
#else
        return -1;
#endif
}

void XTPVulkan::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    function(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

template<class T>
AllocatedBuffer XTPVulkan::createSimpleBuffer(std::vector<T> data, const VkBufferUsageFlags usage,
    const VmaMemoryUsage memoryUsage, const bool record) {
    return createSimpleBuffer(sizeof(data[0]) * data.size(), usage, memoryUsage, record);
}

AllocatedBuffer XTPVulkan::createSimpleBuffer(const VkDeviceSize bufferSize, const VkBufferUsageFlags usage,
    const VmaMemoryUsage memoryUsage, const bool record) {

    VkBufferCreateInfo bufferInfo {};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    AllocatedBuffer buffer {};
    if(vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo, &buffer.internalBuffer, &buffer.allocation, &buffer.info) != VK_SUCCESS) {
        logger->logCritical("Unable To Create Buffer!");
    }
    buffer.usage = usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ? VERTEX_BUFFER: usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT ? INDEX_BUFFER: UNSUPPORTED;

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo addressInfo {};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        addressInfo.buffer = buffer.internalBuffer;

        buffer.gpuAddress = vkGetBufferDeviceAddress(device, &addressInfo);
    }

    if (record) {
        buffers.emplace_back(buffer);
    }

    return buffer;
}

void XTPVulkan::destroyAllocatedBuffer(AllocatedBuffer *buffer) {
    if (buffer->alive) {
        vmaDestroyBuffer(allocator, buffer->internalBuffer, buffer->allocation);
    } else {
        logger->logTrace("Attempted To Destroy Already Destroyed Buffer!");
    }
    buffer->alive = false;
}

template<class T>
AllocatedBuffer XTPVulkan::createBufferWithDataStaging(const std::vector<T> &data, const VkBufferUsageFlagBits usage) {
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

void XTPVulkan::copyBuffer(AllocatedBuffer srcBuffer, AllocatedBuffer dstBuffer, VkDeviceSize size) {
    immediateSubmit([size, srcBuffer, dstBuffer](const VkCommandBuffer& cmd) {
        VkBufferCopy copyRegion {};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, srcBuffer.internalBuffer, dstBuffer.internalBuffer, 1, &copyRegion);
    });
}

AllocatedImage XTPVulkan::createErrorTex() {
    const uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    const uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));

    std::array<uint32_t, 16 * 16> pixels; //for 16x16 checkerboard texture
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }
    AllocatedImage errorImage = createImage(pixels.data(), 16 * 16 * 4, VK_FORMAT_R8G8B8A8_UNORM, 16, 16);
    createSampler(errorImage);
    return errorImage;
}

void XTPVulkan::createImageView(AllocatedImage &image, const VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &image.imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

AllocatedImage XTPVulkan::createImage(const char *path, VkFormat imageFormat) {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * 4 /*This Value is 4, Since Our Format Is STBI_rgb_alpha*/;

    if (!pixels) {
        logger->logError("Failed To Load Texture", !initializedErrorTex);
        return errorTexure;
    }

    AllocatedImage image = createImage(pixels, imageSize, imageFormat, texWidth, texHeight);

    stbi_image_free(pixels);
    return image;
}

AllocatedImage XTPVulkan::createDepthImage() {
    constexpr VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    return createImage(swapchainExtent.width, swapchainExtent.height, depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

AllocatedImage XTPVulkan::createImage(const uint32_t width, const uint32_t height, const VkFormat imageFormat, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags) {
    AllocatedImage image {};

    VkImageCreateInfo imageInfo {};

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (vmaCreateImage(allocator, &imageInfo, &allocationInfo, &image.image, &image.allocation,
                       &image.allocationInfo) != VK_SUCCESS) {
        logger->logError("Failed To Create Image!", initializedErrorTex);
        return errorTexure;
                       }
    createImageView(image, imageFormat, aspectFlags);

    return image;
}

AllocatedImage XTPVulkan::createImage(void *pixels, VkDeviceSize imageSize, VkFormat imageFormat, uint32_t width,
                                      uint32_t height) {
    AllocatedBuffer stagingBuffer = createSimpleBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                       VMA_MEMORY_USAGE_CPU_ONLY, false);
    memcpy(stagingBuffer.info.pMappedData, pixels, imageSize);

    AllocatedImage image = createImage(width, height, imageFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    allLoadedImages.emplace_back(image);

    immediateSubmit([stagingBuffer, image, width, height, imageFormat](auto cmd) {

        transitionImageLayout(cmd, image, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(cmd, stagingBuffer.internalBuffer, image.image, static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height));

        transitionImageLayout(cmd, image, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    });


    destroyAllocatedBuffer(&stagingBuffer);

    return image;
}

void XTPVulkan::createSampler(AllocatedImage& image, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW, bool useAnisotropy, float maxAnisotropy, VkBorderColor borderColor) {
    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    if (image.image != errorTexure.image) {
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = addressModeU;
        samplerInfo.addressModeV = addressModeV;
        samplerInfo.addressModeW = addressModeW;
        samplerInfo.anisotropyEnable = useAnisotropy;
        samplerInfo.maxAnisotropy = maxAnisotropy == -1 ? gpuProperties.limits.maxSamplerAnisotropy: maxAnisotropy;
        samplerInfo.borderColor = borderColor;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
    } else {
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = true;
        samplerInfo.maxAnisotropy = gpuProperties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
    }

    if (vkCreateSampler(device, &samplerInfo, nullptr, &image.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    samplers.emplace_back(image.sampler);
}

void XTPVulkan::cleanUp() {
    ZoneScopedN("XTPVulkan::cleanUp");
    vkDeviceWaitIdle(device);
    logger->logDebug("Cleaning Up Vulkan");


    for (auto &[shader, materialMap]: toRender) {
        if (!shader->hasDeleted()) {
            shader->cleanUp();
        }
        for (const auto &[material, renderables]: materialMap) {
            material->cleanUp();
            for (const std::shared_ptr<Renderable> &renderable: renderables) {
                if (!renderable->getMesh()->destroyed()) {
                    renderable->getMesh()->destroy();
                }
                renderable->remove();
            }
        }
    }
    for (auto buffer: buffers) {
        destroyAllocatedBuffer(&buffer);
    }
    for (auto image: allLoadedImages) {
        destroyAllocatedImage(&image);
    }
    for (auto sampler : samplers) {
        vkDestroySampler(device, sampler, nullptr);
    }
    toRender.clear();
    allocatorPool->Flip();
    allocatorPool.reset();

#ifdef XTP_USE_IMGUI_UI
    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
    vkDestroyQueryPool(device, timeQueryPool, nullptr);
#endif

    vkDestroyRenderPass(device, renderPass, nullptr);


    for (size_t i = 0; i < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(); i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    cleanupSwapchain();

    logger->logDebug("Destroying Vulkan Surface");
    vkDestroySurfaceKHR(instance, surface, nullptr);

    vmaDestroyAllocator(allocator);
    logger->logDebug("Destroying Vulkan Device");
    vkDestroyDevice(device, nullptr);

#ifdef ISDEBUG
    logger->logDebug("Destroying Debug Utils Messenger");
    destroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
#endif

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

void XTPVulkan::addShader(const std::shared_ptr<ShaderObject> &shader) {
    ZoneScopedN("XTPVulkan::addShader");
    shader->init();
    toRender.insert({shader, {}});
}

void XTPVulkan::addMaterial(const std::shared_ptr<Material> &material, std::shared_ptr<ShaderObject> &shader) {
    ZoneScopedN("XTPVulkan::addMaterial");
    toRender.at(shader).insert({material, {}});
}

void XTPVulkan::addRenderable(const std::shared_ptr<Renderable> &renderable) {
    ZoneScopedN("XTPVulkan::addRenderable");
    if (toRender.count(renderable->getShader()) == 0) {
        toRender.insert({renderable->getShader(), {}});
    }
    if (toRender.at(renderable->getShader()).count(renderable->getMaterial()) == 0) {
        toRender.at(renderable->getShader()).insert({renderable->getMaterial(), {}});
    }
    toRender.at(renderable->getShader()).at(renderable->getMaterial()).emplace_back(renderable);
}

std::vector<VkImageView> XTPVulkan::createImageViews() {
    ZoneScopedN("XTPVulkan::createImageViews");
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
    ZoneScopedN("XTPVulkan::printAvailableDeviceExtensions");
    logger->logDebug("Found " + std::to_string(availableDeviceExtensions.size()) + " Vulkan Device Extensions!");
    for (VkExtensionProperties properties: availableDeviceExtensions) {
        logger->logDebug("Found Vulkan Device Extension '" + std::string(properties.extensionName) + "'!");
    }
}

// ReSharper disable once CppNotAllPathsReturnValue
VkSurfaceKHR XTPVulkan::createSurface() {
    ZoneScopedN("XTPVulkan::createSurface");
    VkSurfaceKHR surface;
    logger->logDebug("Creating Vulkan Surface");
    if (XTPWindowing::windowBackend->createVulkanSurface(instance, &surface))
        return
                surface;

    logger->logCritical("Unable To Create Vulkan Surface");
}


void XTPVulkan::transitionImageLayout(VkCommandBuffer cmd, const AllocatedImage image, VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        cmd,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void XTPVulkan::copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width,
                                  uint32_t height) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        cmd,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void XTPVulkan::destroyAllocatedImage(AllocatedImage *image) {
    vkDestroyImageView(device, image->imageView, nullptr);
    vmaDestroyImage(allocator, image->image, image->allocation);
}

VkDevice XTPVulkan::createLogicalDevice() {
    ZoneScopedN("XTPVulkan::createLogicalDevice");
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

    std::vector<std::string> enabledDeviceExtensions = {};

    for (const auto [extensionName, specVersion]: availableDeviceExtensions) {
        if (strcmp(extensionName, "VK_KHR_portability_subset") == 0) {
            enabledDeviceExtensions = {"VK_KHR_portability_subset"};
        }
    }

    enabledDeviceExtensions.emplace_back("VK_KHR_swapchain");

    for (const std::string &extension: VulkanRenderInfo::INSTANCE->getRequiredDeviceExtensions()) {
        enabledDeviceExtensions.emplace_back(extension.c_str());
    }

    for (const std::string &enabledDeviceExtension: enabledDeviceExtensions) {
        bool isPresent = false;
        for (const auto [extensionName, specVersion]: getVkDeviceExtensionInfo()) {
            if (extensionName == enabledDeviceExtension) {
                isPresent = true;
                break;
            }
        }

        if (!isPresent) {
            logger->logCritical("Unable To Find The Required Device Extension '" + enabledDeviceExtension + "'!",
                                false);
        }
    }


    const VkPhysicalDeviceFeatures features = VulkanRenderInfo::INSTANCE->getPhysicalDeviceFeatures();

    std::vector<const char *> cstrVec;
    cstrVec.reserve(enabledDeviceExtensions.size()); // Reserve space to avoid multiple reallocations

    for (const auto &str: enabledDeviceExtensions) {
        cstrVec.push_back(str.c_str());
    }

    VkDeviceCreateInfo deviceCreateInfo{};

    VkPhysicalDeviceVulkan12Features fs{};
    fs.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    fs.bufferDeviceAddress = true;

    VkPhysicalDeviceHostQueryResetFeatures resetFeatures;
    resetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
    resetFeatures.hostQueryReset = VK_TRUE;
    resetFeatures.pNext = &fs;

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = infos.data();
    deviceCreateInfo.queueCreateInfoCount = infos.size();
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.ppEnabledExtensionNames = cstrVec.data();
    deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensions.size();
    deviceCreateInfo.pNext = &resetFeatures;

    // For Compatibility With Older Vulkan Implementations That Distinguished Between Instance And Device Validation Layers
    const std::vector<const char *> enabledLayers = VulkanRenderInfo::INSTANCE->getValidationLayers();

#ifdef ISDEBUG
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
#endif


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
    ZoneScopedN("XTPVulkan::createRenderPass");
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = swapchainImageFormat;

    //todo: work with multisampling
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
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

void XTPVulkan::mergeMeshes(const std::vector<std::shared_ptr<Renderable> > &renderables,
                            AllocatedBuffer &mergedVertexBuffer, AllocatedBuffer &mergedIndexBuffer) {
    ZoneScopedN("XTPVulkan::mergeMeshes");
    uint32_t total_vertices = 0;

    uint32_t total_indices = 0;

    const std::vector<Mesh> meshes;

    for (auto &m: renderables) {
        Mesh mesh{};
        mesh.firstIndex = total_indices;
        mesh.firstVertex = total_vertices;

        total_vertices += m->getMesh()->getVertexCount();
        total_indices += m->getMesh()->getIndexCount();
    }

    constexpr uint32_t vertexSize = sizeof(renderables[0]->getMesh()->getFirstVertex());

    mergedVertexBuffer = createSimpleBuffer(total_vertices * vertexSize,
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);

    mergedIndexBuffer = createSimpleBuffer(total_indices * sizeof(uint32_t),
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VMA_MEMORY_USAGE_GPU_ONLY);

    immediateSubmit([&](VkCommandBuffer cmd) {
        for (int i = 0; i < meshes.size(); ++i) {
            const Mesh mesh = meshes[i];
            Renderable *renderable = renderables[i].get();

            VkBufferCopy vertexCopy;
            vertexCopy.dstOffset = mesh.firstVertex * vertexSize;
            vertexCopy.size = renderable->getMesh()->getVertexCount() * vertexSize;
            vertexCopy.srcOffset = 0;

            vkCmdCopyBuffer(cmd, renderable->getMesh()->getVertexBuffer().internalBuffer,
                            mergedVertexBuffer.internalBuffer, 1, &vertexCopy);

            VkBufferCopy indexCopy;
            indexCopy.dstOffset = mesh.firstIndex * sizeof(uint32_t);
            indexCopy.size = renderable->getMesh()->getIndexCount() * sizeof(uint32_t);
            indexCopy.srcOffset = 0;

            vkCmdCopyBuffer(cmd, renderable->getMesh()->getIndexBuffer().internalBuffer,
                            mergedIndexBuffer.internalBuffer, 1, &indexCopy);
        }
    });
}

VkSwapchainKHR XTPVulkan::createSwapchain() {
    ZoneScopedN("XTPVulkan::createSwapchain");
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
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
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
    ZoneScopedN("XTPVulkan::createFramebuffers");
    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapchainImageViews[i],
            depthImage.imageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;


        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            logger->logCritical("Failed To Create Framebuffer!");
        }
    }
}

void XTPVulkan::updateGlobalMatrices() {
    Camera::camera->updateCamera();
    if (shouldUpdateProjectionMatrix || Camera::camera->shouldUpdateViewMatrix) {
        if (shouldUpdateProjectionMatrix) {
            projectionMatrix = calculateProjectionMatrix(VulkanRenderInfo::INSTANCE->getFOV(),
                                                         VulkanRenderInfo::INSTANCE->getZNear(),
                                                         VulkanRenderInfo::INSTANCE->getZFar());
            shouldUpdateProjectionMatrix = false;
        }
        if (Camera::camera->shouldUpdateViewMatrix) {
            viewMatrix = Camera::camera->createViewMatrix();
            Camera::camera->shouldUpdateViewMatrix = false;
        }

        if (doesSceneBufferNeedToBeUpdated.size() < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight()) {
            doesSceneBufferNeedToBeUpdated.resize(VulkanRenderInfo::INSTANCE->getMaxFramesInFlight());
        }
        for (int i = 0; i < VulkanRenderInfo::INSTANCE->getMaxFramesInFlight(); ++i) {
            doesSceneBufferNeedToBeUpdated[i] = true;
        }
    }
}

void XTPVulkan::render() {
    ZoneScopedN("XTPVulkan::render");
    XTPWindowing::windowBackend->beginFrame();
    updateGlobalMatrices();
    drawFrame();
    FrameMark;
    XTPWindowing::windowBackend->endFrame();
    TimeManager::endFrame();
    currentFrameIndex = (currentFrameIndex + 1) % VulkanRenderInfo::INSTANCE->getMaxFramesInFlight();
}

bool XTPVulkan::hasInitialized() {
    return initialized;
}

VkPresentModeKHR XTPVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    ZoneScopedN("XTPVulkan::chooseSwapPresentMode");
    for (const VkPresentModeKHR &presentMode: availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D XTPVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    ZoneScopedN("XTPVulkan::chooseSwapExtent");
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    XTPWindowing::windowBackend->getFramebufferSize(&width, &height);

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
    ZoneScopedN("XTPVulkan::querySwapChainSupport");
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
    ZoneScopedN("XTPVulkan::chooseSwapSurfaceFormat");
    logger->logDebug("Choosing Swap Surface Format!");
    for (const VkSurfaceFormatKHR availableFormat: availableFormats) {
        logger->logDebug("Found Surface Format " + VkFormatParser::toStringVkFormat(availableFormat.format));
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

std::vector<VkExtensionProperties> XTPVulkan::getVkDeviceExtensionInfo() {
    ZoneScopedN("XTPVulkan::getVkDeviceExtensionInfo");
    VkPhysicalDevice device = gpu;

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> properties(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, properties.data());

    return properties;
}

std::vector<std::string> XTPVulkan::getVkDeviceExtensionNames(VkPhysicalDevice device) {
    ZoneScopedN("XTPVulkan::getVkDeviceExtensionNames");
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
    ZoneScopedN("XTPVulkan::getVkDeviceExtensionCount");
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    return extensionCount;
}

VkPhysicalDevice XTPVulkan::pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices) {
    ZoneScopedN("XTPVulkan::pickPhysicalDevice");
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
    ZoneScopedN("XTPVulkan::getAllPhysicalDevices");
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
    ZoneScopedN("XTPVulkan::getDeviceScore");
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
    ZoneScopedN("XTPVulkan::isDeviceSuitable");
    const auto [capabilities, formats, presentModes] = querySwapChainSupport(device);
    const bool adequateSwapchain = !formats.empty() && !presentModes.empty();

    if (!adequateSwapchain) {
        logger->logDebug("Swapchain for device '" + std::string(properties.deviceName) + "' inadequate!");
    }

    return indices.isComplete() && adequateSwapchain;
}

void XTPVulkan::getDeviceInfo(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties &properties,
                              VkPhysicalDeviceFeatures &features) {
    ZoneScopedN("XTPVulkan::getDeviceInfo");
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    properties = deviceProperties;
    features = deviceFeatures;
}

QueueFamilyIndices
XTPVulkan::findQueueFamilies(VkPhysicalDevice device, const std::string &deviceName, bool printDebug) {
    ZoneScopedN("XTPVulkan::findQueueFamilies");
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
                    if (printDebug)
                        logger->logDebug(
                            "Found New Best Graphics Queue Family (#" + std::to_string(graphicsScore) + ") For Device '"
                            +
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
                    if (printDebug)
                        logger->logDebug(
                            "Found New Best Present Queue Family (#" + std::to_string(i) + ") For Device '" + deviceName
                            +
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
    ZoneScopedN("XTPVulkan::getQueueCaps");
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
    ZoneScopedN("XTPVulkan::debugCallback");
    VulkanRenderInfo::INSTANCE->onVulkanDebugMessage(messageSeverity, messageType, pCallbackData, pUserData,
                                                     std::string(pCallbackData->pMessage));

    return 0;
}

VkResult XTPVulkan::createDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger) {
    ZoneScopedN("XTPVulkan::createDebugUtilsMessengerEXT");
    if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// ReSharper disable once CppNotAllPathsReturnValue
VkDebugUtilsMessengerEXT XTPVulkan::getDebugMessenger() {
    ZoneScopedN("XTPVulkan::getDebugMessenger");
#ifndef ISDEBUG
        logger->logError("Attempting To Access A Debug Only Function In A Non-Debug Context, Aborting!");
#else
    logger->logDebug("Creating Vulkan Debug Messenger Create Info");
    const VkDebugUtilsMessengerCreateInfoEXT createInfo = getDebugUtilsMessengerCreateInfo();

    if (VkDebugUtilsMessengerEXT messenger; createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &messenger) ==
                                            VK_SUCCESS)
        return messenger;

    logger->logError("Unable to Initialize Vulkan Debug Messenger!");
#endif
}

VkDebugUtilsMessengerCreateInfoEXT XTPVulkan::getDebugUtilsMessengerCreateInfo() {
    ZoneScopedN("XTPVulkan::getDebugUtilsMessengerCreateInfo");
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
    return info;
}

VkInstance XTPVulkan::createVulkanInstance() {
    ZoneScopedN("XTPVulkan::createVulkanInstance");

    logger->logDebug("Creating Vulkan Application Info");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = VulkanRenderInfo::INSTANCE->getApplicationName();
    appInfo.applicationVersion = VulkanRenderInfo::INSTANCE->getApplicationVersion();
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;

    logger->logDebug("Creating Vulkan Instance Create Info");
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.pNext = &features;

    VkInstanceCreateFlags flags{};
    const std::vector<std::string> enabledExtensions = findAllVulkanInstanceExtensions(flags);

    std::vector<const char *> cstrVec;
    cstrVec.reserve(enabledExtensions.size()); // Reserve space to avoid multiple reallocations

    for (const auto &str: enabledExtensions) {
        cstrVec.push_back(str.c_str());
    }

    const std::vector enabledLayers = VulkanRenderInfo::INSTANCE->getValidationLayers();

#ifdef ISDEBUG
    if (!checkForValidationLayerSupport()) {
        logger->logError("Could Not Find Requested Validation Layers!", false);
    } else {
        info.enabledLayerCount = enabledLayers.size();
        info.ppEnabledLayerNames = enabledLayers.data();
    }
#endif

    info.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    info.ppEnabledExtensionNames = cstrVec.data();
    info.flags = flags;

    logger->logDebug("Creating Vulkan Instance");

    VkInstance instance;
    if (const VkResult result = vkCreateInstance(&info, nullptr, &instance); result != VK_SUCCESS) {
        logger->logCritical("Failed To Create Vulkan Instance!");
        throw std::runtime_error("Unable To Initialize Vulkan");
    }

    logger->logDebug("Created Vulkan Instance With The Following Extensions: " + makeListOf(cstrVec));
#ifdef ISDEBUG
    logger->logDebug("Created Vulkan Instance With The Following Validation Layers: " + makeListOf(enabledLayers));
#endif

    return instance;
}

void XTPVulkan::printAvailableVulkanProperties() {
    ZoneScopedN("XTPVulkan::printAvailableVulkanProperties");
    for (auto [extensionName, specVersion]: availableInstanceExtensions) {
        logger->logDebug("Found Vulkan Instance Extension '" + std::string(extensionName) + "'!");
    }

    for (auto [layerName, specVersion, implementationVersion, description]: availableValidationLayers) {
        logger->logDebug("Found Vulkan Validation Layer '" + std::string(layerName) + "'!");
    }
}

bool XTPVulkan::checkForValidationLayerSupport() {
    ZoneScopedN("XTPVulkan::checkForValidationLayerSupport");
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    availableValidationLayers = availableLayers;

    return ensureAllValidationLayersPresent(availableLayers);
}

bool XTPVulkan::ensureAllValidationLayersPresent(const std::vector<VkLayerProperties> &availableLayers) {
    ZoneScopedN("XTPVulkan::ensureAllValidationLayersPresent");
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
        logger->logError("Could Not Find Requested Vulkan Validation Layer '" + validationLayer + "'!", false);
    }

    return !hasEncounteredMissingLayer;
}

std::string XTPVulkan::getLayerName(const VkLayerProperties &layerProperties) {
    ZoneScopedN("XTPVulkan::getLayerName");
    return layerProperties.layerName;
}

std::vector<std::string> XTPVulkan::findAllVulkanInstanceExtensions(VkInstanceCreateFlags &flags) {
    ZoneScopedN("XTPVulkan::findAllVulkanInstanceExtensions");
    std::vector<std::string> enabledInstanceExtensions;
    VkInstanceCreateFlags createFlags;
    setupAdditionalVulkanInstanceExtensions(enabledInstanceExtensions, createFlags);

    uint32_t extensionCount;
    const char **glfwInstanceExtensions = XTPWindowing::windowBackend->
            getRequiredInstanceExtensions(extensionCount);
    for (int i = 0; i < extensionCount; i++) {
        enabledInstanceExtensions.emplace_back(glfwInstanceExtensions[i]);
    }

    flags = createFlags;

    for (const std::string &enabledInstanceExtension: enabledInstanceExtensions) {
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
    ZoneScopedN("XTPVulkan::getExtensionName");
    return (extension.extensionName);
}

void XTPVulkan::setupAdditionalVulkanInstanceExtensions(std::vector<std::string> &enabledInstanceExtensions,
                                                        VkInstanceCreateFlags &flags) {
    ZoneScopedN("XTPVulkan::setupAdditionalVulkanInstanceExtensions");
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

#ifdef ISDEBUG
    enabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
}

std::vector<VkExtensionProperties> XTPVulkan::getVkInstanceExtensionInfo() {
    ZoneScopedN("XTPVulkan::getVkInstanceExtensionInfo");
    uint32_t extensionCount = getVkInstanceExtensionCount();

    logger->logDebug("Found " + std::to_string(extensionCount) + " Vulkan Instance Extensions!");

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

    return extensionProperties;
}

uint32_t XTPVulkan::getVkInstanceExtensionCount() {
    ZoneScopedN("XTPVulkan::getVkInstanceExtensionCount");
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    return extensionCount;
}

void XTPVulkan::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pAllocator) {
    ZoneScopedN("XTPVulkan::destroyDebugUtilsMessengerEXT");
    if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")); func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

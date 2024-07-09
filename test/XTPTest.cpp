#include "Events.h"
#include "SimpleLogger.h"
#include "XTP.h"
#include "XTPWindowing.h"
#include "GLFWVulkanWindowing.h"
#include "ShaderRegisterEvent.h"
#include "../src/vulkan/renderable/SimpleIndexBufferedRenderable.h"
#include "TestShaderObject.h"
#include "TestWindowData.h"
#include "VulkanRenderInfo.h"
#include "XTPVulkan.h"
#include "XTPTest.h"

std::string XTPTest::executablePath;
SimpleLogger XTPTest::testLogger = {"Test Logger", INFORMATION};
std::shared_ptr<ShaderObject> XTPTest::testShader = nullptr;

class TestShaderEvent final : public ShaderRegisterEvent {
    void onRegisterShaders() override {
        XTPTest::testShader = std::shared_ptr<ShaderObject>(new TestShaderObject(
            XTPTest::executablePath + "/shaders/test.vert", XTPTest::executablePath + "./shaders/test.frag", {.renderPass = XTPVulkan::getMainRenderPass()}));
        XTPVulkan::addShader(XTPTest::testShader);

        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };
        
        XTPVulkan::addRenderable(std::unique_ptr<Renderable>(new SimpleIndexBufferedRenderable(vertices, indices, XTPTest::testShader)));
    }
};

int main(const int argc, char *argv[]) {
    XTPTest::executablePath = std::string(std::filesystem::absolute(argv[0]).remove_filename());
    XTPWindowing::setWindowBackend(std::unique_ptr<XTPWindowBackend>(new GLFWVulkanWindowing()));
    XTPWindowing::setWindowData(std::unique_ptr<XTPWindowData>(new TestWindowData));
    VulkanRenderInfo::INSTANCE = new VulkanRenderInfo();

    Events::registerEvent(std::unique_ptr<Event>(new TestShaderEvent()));
    XTP::start(std::chrono::milliseconds{1});

    return 0;
}

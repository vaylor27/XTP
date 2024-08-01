#ifdef XTP_USE_IMGUI_UI
#include "TestImGuiRenderer.h"
#include "implot.h"
#endif
#include "XTP.h"
#include "Events.h"
#include "SimpleLogger.h"
#include "XTPWindowing.h"
#include "GLFWWindowBackend.h"
#include "ShaderRegisterEvent.h"
#include "renderable/SimpleIndexBufferedRenderable.h"
#include "TestShaderObject.h"
#include "TestWindowData.h"
#include "VulkanRenderInfo.h"

#include "Camera.h"
#include "CleanUpEvent.h"
#include "FrameEvent.h"
#include "InitEvent.h"
#include "RenderDebugUIEvent.h"
#include "TestCamera.h"
#include "TestRenderable.h"
#include "TimeManager.h"
#include "renderable/SimpleMesh.h"

static SimpleLogger testLogger = {"Test Logger", INFORMATION};
static std::shared_ptr<SimpleShaderObject> testShader = nullptr;

class TestShaderEvent final : public ShaderRegisterEvent {
    void onRegisterShaders() override {
        testShader = std::shared_ptr<SimpleShaderObject>(new TestShaderObject(
            "./assets/shaders/test.vert.spv", "./assets/shaders/test.frag.spv", {.cullMode = VK_CULL_MODE_NONE}));
        XTPVulkan::addShader(testShader);

        std::vector<VertexData> vertices = {
            {{}, {-0.5f,0.5f,-0.5f}, {0, 0}},
            {{}, {-0.5f,-0.5f,-0.5f}, {0, 1}},
            {{}, {0.5f,-0.5f,-0.5f}, {1, 1}},
            {{}, {0.5f,0.5f,-0.5f}, {1, 0}},

            {{}, {-0.5f,0.5f,0.5f}, {0, 0}},
            {{}, {-0.5f,-0.5f,0.5f}, {0, 1}},
            {{}, {0.5f,-0.5f,0.5f}, {1, 1}},
            {{}, {0.5f,0.5f,0.5f}, {1, 0}},

            {{}, {0.5f,0.5f,-0.5f}, {0, 0}},
            {{}, {0.5f,-0.5f,-0.5f}, {0, 1}},
            {{}, {0.5f,-0.5f,0.5f}, {1, 1}},
            {{}, {0.5f,0.5f,0.5f}, {1, 0}},

            {{}, {-0.5f,0.5f,-0.5f}, {0, 0}},
            {{}, {-0.5f,-0.5f,-0.5f}, {0, 1}},
            {{}, {-0.5f,-0.5f,0.5f}, {1, 1}},
            {{}, {-0.5f,0.5f,0.5f}, {1, 0}},

            {{}, {-0.5f,0.5f,0.5f}, {0, 0}},
            {{}, {-0.5f,0.5f,-0.5f}, {0, 1}},
            {{}, {0.5f,0.5f,-0.5f}, {1, 1}},
            {{}, {0.5f,0.5f,0.5f}, {1, 0}},

            {{}, {-0.5f,-0.5f,0.5f}, {0, 0}},
            {{}, {-0.5f,-0.5f,-0.5f}, {0, 1}},
            {{}, {0.5f,-0.5f,-0.5f}, {1, 1}},
            {{}, {0.5f,-0.5f,0.5f}, {1, 0}}
    };

        std::vector<uint32_t> indices = {
            0,1,3,
            3,1,2,
            4,5,7,
            7,5,6,
            8,9,11,
            11,9,10,
            12,13,15,
            15,13,14,
            16,17,19,
            19,17,18,
            20,21,23,
            23,21,22

    };

        // const std::vector<VertexData> vertices = {
        //     // {{}, {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        //     // {{}, {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        //     // {{}, {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        //     // {{}, {-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        //
        //     {{}, {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        //     {{}, {0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        //     {{}, {0.5f,
        //         0.5f, -0.5f}, {1.0f, 1.0f}},
        //     {{}, {-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}
        // };
        //
        // const std::vector<uint32_t> indices = {
        //     0, 1, 2, 2, 3, 0,
        //     4, 5, 6, 6, 7, 4
        // };

        XTPVulkan::addRenderable(std::unique_ptr<Renderable>(new TestRenderable(testShader, std::make_shared<SimpleMesh<VertexData>>(vertices, indices))));
    }
};

class TestDebugUIEvent final : public RenderDebugUIEvent {
    void renderDebugUI() override {
#ifdef XTP_USE_IMGUI_UI
        TestImGuiRenderer::render();
#endif
    }

};

class TestKeyPressEvent final: public KeyPressEvent {
    void onKeyPressed(int key, int action, int mods) override {
        if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
            if (XTPWindowing::windowBackend->isMouseCaptured()) {
                XTPWindowing::windowBackend->releaseMouse(true);
            } else {
                XTPWindowing::windowBackend->captureMouse();
            }
        }
    }
};

class TestInitEvent final : public InitEvent {
    void onInit() override {
#ifdef XTP_USE_IMGUI_UI
        TestImGuiRenderer::init();
#endif
        XTPWindowing::windowBackend->captureMouse();
    }
};

class TestFrameEvent final: public FrameEvent {
public:
    void onFrameBegin() override {}

    void onFrameDrawBegin() override {}

    void onFrameDrawEnd() override {}

    void onFrameEnd() override {
    }
};

class TestCleanUpEvent final : public CleanUpEvent {
    void cleanUp() override {
#ifdef XTP_USE_IMGUI_UI
        TestImGuiRenderer::cleanUp();
#endif
    }
};

int main(const int argc, char *argv[]) {
    XTPWindowing::setWindowBackend(std::unique_ptr<XTPWindowBackend>(new GLFWWindowBackend()));
    XTPWindowing::setWindowData(std::unique_ptr<XTPWindowData>(new TestWindowData));
    VulkanRenderInfo::INSTANCE = new VulkanRenderInfo();
    Camera::camera = std::shared_ptr<Camera>(new TestCamera());

    Events::registerEvent(std::unique_ptr<Event>(new TestShaderEvent()));
    Events::registerEvent(std::unique_ptr<Event>(new TestDebugUIEvent()));
    Events::registerEvent(std::unique_ptr<Event>(new TestInitEvent()));
    Events::registerEvent(std::unique_ptr<Event>(new TestKeyPressEvent()));
    Events::registerEvent(std::unique_ptr<Event>(new TestCleanUpEvent()));
    Events::registerEvent(std::unique_ptr<Event>(new TestFrameEvent()));
    XTP::start(std::chrono::milliseconds{1});

    return 0;
}

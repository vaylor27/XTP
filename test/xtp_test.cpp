#include "SimpleLogger.h"
#include "XTP.h"
#include "XTPWindowing.h"
#include "GLFWVulkanWindowing.h"
#include "TestWindowData.h"
#include "VulkanRenderInfo.h"
#include "XTPRendering.h"
#include "XTPVulkan.h"

static SimpleLogger testLogger("Test Logger", INFORMATION);

int main() {
    testLogger.logInformation("Starting Test App!");
    XTPWindowing::windowBackend = reinterpret_cast<XTPWindowBackend*>(new GLFWVulkanWindowing());
    XTPWindowing::data = dynamic_cast<XTPWindowData*>(new TestWindowData());
    XTPRendering::renderInfo = dynamic_cast<XTPRenderInfo*>(new VulkanRenderInfo());
    XTPRendering::renderBackend = dynamic_cast<XTPRenderBackend*>(new XTPVulkan());
    XTP::start();
    testLogger.logInformation("Finished Test App Execution!");

    return 0;
}

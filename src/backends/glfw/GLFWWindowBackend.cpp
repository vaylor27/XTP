
#include "GLFWWindowBackend.h"

GLFWwindow* GLFWWindowBackend::window = nullptr;
std::unordered_map<uint32_t, bool> GLFWWindowBackend::keys;
glm::dvec2 GLFWWindowBackend::scrollDelta;
glm::dvec2 GLFWWindowBackend::mouseDelta;
glm::dvec2 GLFWWindowBackend::mousePos;
glm::dvec2 GLFWWindowBackend::lastReleasedMousePos;
bool GLFWWindowBackend::initializedMousePos = false;
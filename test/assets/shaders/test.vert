#version 450

//layout(set = 0, binding = 0) uniform GlobalData {
//    mat4 projectionMatrix;
//    mat4 viewMatrix;
//} global;
//
//layout(set = 1, binding = 0) uniform ObjectData {
//    mat4 transformMatrix;
//} obj;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
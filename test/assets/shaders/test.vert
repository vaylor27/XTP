#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : enable

layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer GlobalData
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer ObjectData
{
    mat4 transformationMatrix;
};

layout(push_constant, std430) uniform Data
{
    GlobalData global;
    ObjectData object;
} data;

//Mesh Data
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {

    gl_Position = data.global.projectionMatrix * data.global.viewMatrix * data.object.transformationMatrix * vec4(inPosition, 1.0);
//    debugPrintfEXT("Pos:%1.2v4f", gl_Position);
    fragTexCoord = inTexCoord;
}


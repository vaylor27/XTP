#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
    if (outColor.a <= 0.01) {
        discard;
    }
}
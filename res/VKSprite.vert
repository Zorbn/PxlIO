#version 450

precision highp float;

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inTint;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float fragTint;

void main()
{
    gl_Position = ubo.proj * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTint = inTint;
}

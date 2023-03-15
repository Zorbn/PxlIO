#version 450

precision highp float;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float fragTint;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texColor = texture(texSampler, fragTexCoord);
    texColor = vec4(mix(texColor.rgb, fragColor.rgb, fragTint), texColor.a * fragColor.a);

    if (texColor.a == 0.0)
    {
        discard;
    }

    outColor = texColor;
}
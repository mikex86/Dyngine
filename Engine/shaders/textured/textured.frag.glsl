#version 450 core

layout(binding = 0) uniform sampler albedoSampler;
layout(binding = 1) uniform texture2D albedoTexture;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = dot(vNormal, lightDir);
    vec4 albedo = texture(sampler2D(albedoTexture, albedoSampler), vTexCoord);
    fragColor = vec4(albedo.rgb * brightness, 1.0);
}
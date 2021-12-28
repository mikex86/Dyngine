#version 450

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D rmaSampler;

struct Light
{
    vec3 position;
    vec3 color;
    float intensity;
};

layout(std140, binding = 4) uniform LightData
{
    Light pointLights[];
};

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;

layout(location = 0) out vec4 fragColor;


void main()
{
    vec3 lightDir = vec3(1.0f, 0.0f, 0.0f);

    vec4 albedo = texture(albedoSampler, vTexCoord);
    vec3 tNormal = texture(normalSampler, vTexCoord).xyz;
    vec3 normal = normalize(vNormal + tNormal);
    vec3 rma = texture(rmaSampler, vTexCoord).xyz;

    float roughness = rma.x;
    float metalness = rma.y;
    float ao = 1 - rma.z;

    fragColor = vec4(albedo.rgb, albedo.a);
}
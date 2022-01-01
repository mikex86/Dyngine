#version 450

layout(std140, binding = 0) uniform CameraShaderState
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 modelMatrix;

    vec3 cameraPosition; // padded to 16 bytes
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec2 vTexCoord;
layout(location = 3) out mat3 TBN;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 screenPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    gl_Position = screenPos;
    vPos = position;
    vec3 vNormal = normalize(normal);
    vec3 vTangent = normalize(tangent.xyz);
    vec3 vBitangent = cross(vNormal, vTangent);
    TBN = mat3(vTangent, vBitangent, vNormal);
    vTexCoord = texCoord;
}
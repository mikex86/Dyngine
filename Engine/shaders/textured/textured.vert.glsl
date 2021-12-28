#version 450

layout(std140, binding = 0) uniform CameraShaderState
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 modelMatrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec4 color;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 vColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 worldPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    gl_Position = worldPos;
    vNormal = normalize(normal);
    vTexCoord = texCoord;
    vColor = color;
}
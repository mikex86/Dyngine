#version 450 core

layout(std140, binding = 1) uniform CameraShaderState
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 modelMatrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 vertexColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 worldPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    gl_Position = worldPos;
    vertexColor = color;
}
#version 450 core

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = dot(vNormal, lightDir);
    fragColor = vec4(brightness, brightness, brightness, 1.0);
}
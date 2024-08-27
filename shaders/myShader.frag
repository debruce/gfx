#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359;
const float RECIPROCAL_PI = 0.31830988618;
const float RECIPROCAL_PI2 = 0.15915494;
const float EPSILON = 1e-6;
const float c_MinRoughness = 0.04;

layout(location = 0) in vec3 eyePos;
layout(location = 1) in vec3 normalDir;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec3 viewDir;


layout(location = 0) out vec4 color;

void main()
{
    color = vertexColor;
}

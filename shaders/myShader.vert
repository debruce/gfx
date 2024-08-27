#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
};

layout(set = 0, binding = 0) uniform Params {
    vec4 color;
} parms;

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec4 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    gl_Position = (projection * modelView) * vec4(vertex, 1.0);
    fragColor = parms.color;
    gl_PointSize = 3.0;
}

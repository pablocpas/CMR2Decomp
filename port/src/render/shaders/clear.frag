#version 450

layout(location = 0) out vec4 outColor;

layout(std140, set = 3, binding = 0) uniform ClearUniforms {
    vec4 color;
};

void main()
{
    outColor = color;
}

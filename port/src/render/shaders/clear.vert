#version 450
// A triangle covering the viewport, at the clear depth.

layout(std140, set = 1, binding = 0) uniform ClearUniforms {
    vec4 color;
    vec4 depth;
};

void main()
{
    vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(p * 2.0 - 1.0, depth.x, 1.0);
}

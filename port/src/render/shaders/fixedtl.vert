#version 450
#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout(location = 0) in vec4 inPosition;   // screen x, y, z, 1/w
layout(location = 1) in vec4 inDiffuse;    // B, G, R, A bytes
layout(location = 2) in vec4 inSpecular;
layout(location = 3) in vec2 inTex;

layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec4 outTex0;
layout(location = 3) out vec4 outTex1;
layout(location = 4) out float outFog;
layout(location = 5) out float outEyeDepth;

void main()
{
    float w = inPosition.w != 0.0 ? 1.0 / inPosition.w : 1.0;
    // Direct3D 7 puts pixel centres on integer coordinates: half a pixel
    // to the right and down of where they are now.
    float x = (inPosition.x + 0.5) / target.x * 2.0 - 1.0;
    float y = 1.0 - (inPosition.y + 0.5) / target.y * 2.0;
    gl_Position = vec4(x * w, y * w, inPosition.z * w, w);
    outDiffuse = inDiffuse.zyxw;
    outSpecular = inSpecular.zyxw;
    outTex0 = vec4(inTex, 1.0, 0.0);
    outTex1 = vec4(inTex, 1.0, 0.0);
    // Pre-transformed vertices carry their fog factor in specular alpha.
    outFog = inSpecular.w;
    outEyeDepth = w;
}

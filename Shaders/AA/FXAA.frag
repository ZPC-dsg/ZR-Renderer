#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 1) in vec2 texCoord;

// Clamp to edge，并使用点采样
layout (binding = 0) uniform sampler2D sceneColor;

uniform float luma_threshold;
uniform float luma_threshold_min;

float Luma(vec3 linear_color)
{
    vec3 coeffs = vec3(0.2126, 0.7152, 0.0722);
    return dot(coeffs, linear_color);
}

void main()
{
    vec2 C = texCoord;
    
    vec2 tex_size = vec2(textureSize(sceneColor, 0));
    vec2 L = texCoord - (1.0 / tex_size.x, 0.0);
    vec2 R = texCoord + (1.0 / tex_size.x, 0.0);
    vec2 U = texCoord + (0.0, 1.0 / tex_size.y);
    vec2 D = texCoord - (0.0, 1.0 / tex_size.y);

    vec4 colorC = texture(sceneColor, C);
    vec4 colorL = texture(sceneColor, L);
    vec4 colorR = texture(sceneColor, R);
    vec4 colorU = texture(sceneColor, U);
    vec4 colorD = texture(sceneColor, D);

    float lumaC = Luma(colorC.rgb);
    float lumaL = Luma(colorL.rgb);
    float lumaR = Luma(colorR.rgb);
    float lumaU = Luma(colorU.rgb);
    float lumaD = Luma(colorD.rgb);

    float lumaRangeMax = max(lumaC, max(max(lumaL, lumaR), max(lumaU, lumaD)));
    float lumaRangeMin = min(lumaC, min(min(lumaL, lumaR), min(lumaU, lumaD)));
    float lumaRange = lumaRangeMax - lumaRangeMin;

    if (lumaRange < max(luma_threshold_min, luma_threshold * lumaRangeMax))
    {
        FragColor = vec4(colorC.rgb, 1.0);
    }
}
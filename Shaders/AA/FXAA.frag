#version 450 core

#define FXAA_STEP_NUM 12
const float SampleDistances[FXAA_STEP_NUM] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

// Clamp to edge，并使用点采样
layout (binding = 2) uniform sampler2D sceneColor_point;
layout (binding = 0) uniform sampler2D sceneColor_linear;

uniform float luma_threshold;
uniform float luma_threshold_min;
uniform float subpixel_quality;

float Luma(vec3 linear_color)
{
    vec3 coeffs = vec3(0.2126, 0.7152, 0.0722);
    return dot(coeffs, linear_color);
}

void main()
{
    vec2 C = texCoord;

    vec2 tex_size = vec2(textureSize(sceneColor_point, 0));
    float x_offset = 1.0 / tex_size.x;
    float y_offset = 1.0 / tex_size.y;
    
    vec2 L = texCoord - vec2(x_offset, 0.0);
    vec2 R = texCoord + vec2(x_offset, 0.0);
    vec2 U = texCoord + vec2(0.0, y_offset);
    vec2 D = texCoord - vec2(0.0, y_offset);
    vec2 LU = texCoord + vec2(-x_offset, y_offset);
    vec2 RU = texCoord + vec2(x_offset, y_offset);
    vec2 RD = texCoord + vec2(x_offset, -y_offset);
    vec2 LD = texCoord + vec2(-x_offset, -y_offset);

    vec4 colorC = texture(sceneColor_point, C);
    vec4 colorL = texture(sceneColor_point, L);
    vec4 colorR = texture(sceneColor_point, R);
    vec4 colorU = texture(sceneColor_point, U);
    vec4 colorD = texture(sceneColor_point, D);

    float lumaC = Luma(colorC.rgb);
    float lumaL = Luma(colorL.rgb);
    float lumaR = Luma(colorR.rgb);
    float lumaU = Luma(colorU.rgb);
    float lumaD = Luma(colorD.rgb);
    
    float lumaRangeMax = max(lumaC, max(max(lumaL, lumaR), max(lumaU, lumaD)));
    float lumaRangeMin = min(lumaC, min(min(lumaL, lumaR), min(lumaU, lumaD)));
    float lumaRange = lumaRangeMax - lumaRangeMin;

    // 如果亮度变化低于一个与最大亮度呈正相关的阈值，或者低于一个绝对阈值，说明不是处于边缘区域，不进行任何抗锯齿操作
    if (lumaRange < max(luma_threshold_min, luma_threshold * lumaRangeMax))
    {
        FragColor = vec4(colorC.rgb, 1.0);
        return;
    }

    vec4 colorLU = texture(sceneColor_point, LU);
    vec4 colorRU = texture(sceneColor_point, RU);
    vec4 colorRD = texture(sceneColor_point, RD);
    vec4 colorLD = texture(sceneColor_point, LD);

    float lumaLU = Luma(colorLU.rgb);
    float lumaRU = Luma(colorRU.rgb);
    float lumaRD = Luma(colorRD.rgb);
    float lumaLD = Luma(colorLD.rgb);

    float lumaLR = lumaL + lumaR;
    float lumaUD = lumaU + lumaD;
    float lumaLURU = lumaLU + lumaRU;
    float lumaRURD = lumaRU + lumaRD;
    float lumaRDLD = lumaRD + lumaLD;
    float lumaLDLU = lumaLD + lumaLU;

    float edgeHorz = abs(lumaLDLU - 2.0 * lumaL) + 2.0 * abs(lumaUD - 2.0 * lumaC) + abs(lumaRURD - 2.0 * lumaR);
    float edgeVert = abs(lumaLURU - 2.0 * lumaU) + 2.0 * abs(lumaLR - 2.0 * lumaC) + abs(lumaRDLD - 2.0 * lumaD);

    // 判断边界是否水平
    bool is_horz = edgeHorz >= edgeVert;

    float luma1 = is_horz ? lumaU : lumaR;
    float luma2 = is_horz ? lumaD : lumaL;
    float gradient1 = abs(luma1 - lumaC);
    float gradient2 = abs(luma2 - lumaC);

    // 判断边界哪边是内部
    bool is1_steeper = gradient1 >= gradient2;

    float gradientScaled = max(gradient1, gradient2) * 0.25f;

    vec2 offset = is_horz ? vec2(0.0, y_offset * 0.5) : vec2(x_offset * 0.5, 0.0);
    vec2 B = C + (is1_steeper ? offset : -offset);
    float lumaAvg = 0.5 * (is1_steeper ? luma1 + lumaC : luma2 + lumaC);

    // 开始向边界两边偏移
    offset = is_horz ? vec2(x_offset, 0.0) : vec2(0.0, y_offset);
    vec2 pos1 = B - offset;
    vec2 pos2 = B + offset;

    float lumaEnd1 = Luma(texture(sceneColor_linear, pos1).rgb) - lumaAvg;
    float lumaEnd2 = Luma(texture(sceneColor_linear, pos2).rgb) - lumaAvg;

    bool done1 = abs(lumaEnd1) >= gradientScaled;
    bool done2 = abs(lumaEnd2) >= gradientScaled;
    bool done12 = done1 && done2;

    uint index = 1;
    while (!done12 && index < FXAA_STEP_NUM)
    {
        offset = is_horz ? vec2(SampleDistances[index] * x_offset, 0.0) : vec2(0.0, SampleDistances[index] * y_offset);

        if (!done1)
        {
            pos1 -= offset;
            lumaEnd1 = Luma(texture(sceneColor_linear, pos1).rgb) - lumaAvg;
            done1 = abs(lumaEnd1) >= gradientScaled;

        }
        if (!done2)
        {
            pos2 += offset;
            lumaEnd2 = Luma(texture(sceneColor_linear, pos2).rgb) - lumaAvg;
            done2 = abs(lumaEnd2) >= gradientScaled;
        }

        done12 = done1 && done2;

        index++;
    }

    float dist1 = is_horz ? B.x - pos1.x : B.y - pos1.y;
    float dist2 = is_horz ? pos2.x - B.x : pos2.y - B.y;
    bool direction1 = dist1 < dist2;
    float dist = min(dist1, dist2);

    float span_length = max(dist1 + dist2, is_horz ? y_offset : x_offset);
    float pixel_offset = 0.5 - dist / span_length;

    // 判断这个span是不是合理
    bool is_smaller = lumaC <= lumaAvg;
    bool is_valid = direction1 ? (lumaEnd1 <= 0.0) != is_smaller : (lumaEnd2 <= 0.0) != is_smaller;
    pixel_offset = is_valid ? pixel_offset : 0.0;

    // 亚像素抗锯齿，对比度越大偏移越大
    float sub_pixel = (2.0 * (lumaUD + lumaLR) + lumaLURU + lumaRDLD) / 12.0 - lumaC;
    sub_pixel = clamp(abs(sub_pixel) / lumaRange, 0.0, 1.0);
    sub_pixel = (3.0 - 2.0 * sub_pixel) * pow(sub_pixel, 2.0);
    sub_pixel = pow(sub_pixel, 2.0) * subpixel_quality;
    pixel_offset = max(pixel_offset, sub_pixel);

    vec2 final_offset = is_horz ? vec2(0.0, pixel_offset) * y_offset : vec2(pixel_offset, 0.0) * x_offset;
    final_offset = is1_steeper ? final_offset : -final_offset;

    FragColor = vec4(texture(sceneColor_linear, C + final_offset).rgb, 1.0);
    return;
}
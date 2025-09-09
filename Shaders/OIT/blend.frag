#version 450 core

#define MAX_ARRAY_SIZE 15

out vec4 FragColor;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1) uniform samplerBuffer list_buffer;
layout (binding = 2) uniform sampler2D opaque_image;

uvec4 pixel_array[MAX_ARRAY_SIZE];

vec4 UnpackColor(uint packedColor) 
{
    // 提取每个通道的8位值（0-255范围）
    uint r = packedColor & 0xFF;               // 取低8位（R通道）
    uint g = (packedColor >> 8) & 0xFF;        // 右移8位后取低8位（G通道）
    uint b = (packedColor >> 16) & 0xFF;       // 右移16位后取低8位（B通道）
    uint a = (packedColor >> 24) & 0xFF;       // 右移24位后取低8位（A通道）
    
    // 转换为float并归一化到[0.0, 1.0]
    return vec4(
        float(r) / 255.0f,
        float(g) / 255.0f,
        float(b) / 255.0f,
        float(a) / 255.0f
    );
}

// 按照深度从大到小排序
void SortPixelInPlace(uint numPixels)
{
    uvec4 temp;
    for (uint i = 1; i < numPixels; i++)
    {
        for (uint j = i - 1; j > 0; j--)
        {
            float depth_before = uintBitsToFloat(pixel_array[j].z);
            float depth_after = uintBitsToFloat(pixel_array[j + 1].z);

            if (depth_before < depth_after)
            {
                temp = pixel_array[j];
                pixel_array[j] = pixel_array[j + 1];
                pixel_array[j + 1] = temp;
            }
        }
    }
}

void main()
{
    ivec2 index = ivec2(gl_FragCoord.xy);
    uint offset = imageLoad(head_pointer_image, index).x;
    uint fragCount = 0;
    
    while (offset != 0xFFFFFFFF && fragCount < MAX_ARRAY_SIZE)
    {
        uvec4 data = uvec4(texelFetch(list_buffer, int(offset)));
        pixel_array[fragCount] = data;
        offset = data.x;
        fragCount++;
    }

    SortPixelInPlace(fragCount);

    vec4 outColor = vec4(texelFetch(opaque_image, ivec2(gl_FragCoord.xy), 0).rgb, 1.0);
    for (uint i = 0; i < fragCount; i++)
    {
        // vec4 cur_color = UnpackColor(pixel_array[i].y);
        // vec4 cur_color = vec4(0.9, 0.7, 0.8, 0.1);
        vec4 cur_color = vec4(float(i + 1) / float(i + 3), float (i + 4) / float(i + 5), float(i + 1) / float (i * i + 1), 0.1);
        outColor.rgb = mix(outColor.rgb, cur_color.rgb, cur_color.a);
    }

    FragColor = outColor;
}
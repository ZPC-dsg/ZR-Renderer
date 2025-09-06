#version 450 core

in vec4 color;

layout (early_fragment_tests) in;

layout (binding = 0, r32ui) uniform uimage2D head_index_image;
layout (binding = 1, rgba32ui) uniform imageBuffer list_buffer;
layout (binding = 0, offset = 0) uniform atomic_uint node_counter;

uint PackColor(vec4 inColor) 
{
    // 1. 钳位颜色分量到 [0.0, 1.0]，避免超出范围
    vec4 clamped = clamp(inColor, 0.0f, 1.0f);
    
    // 2. 乘以255后加0.5，实现四舍五入（而非截断），再转换为无符号整数
    uvec4 colorUInt4 = uvec4(clamped * 255.0f + 0.5f);
    
    // 3. 按 RGBA 顺序打包到 32 位 uint 中
    return colorUInt4.r | (colorUInt4.g << 8) | (colorUInt4.b << 16) | (colorUInt4.a << 24);
}

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

void main()
{
    uint pixel_count = atomicCounterIncrement(node_counter);

    uint oldStartOffset = imageAtomicExchange(head_index_image, ivec2(gl_FragCoord.xy), pixel_count);

    uvec4 item;
    item.x = oldStartOffset;
    item.y = PackColor(color);
    item.z = floatBitsToUint(gl_FragCoord.z);
    item.w = 0;

    imageStore(list_buffer, pixel_count, item);
}
#version 450 core

layout (early_fragment_tests) in;

layout (location = 0) in vec4 geo_color;

layout (binding = 0, r32ui) uniform uimage2D head_index_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 0, offset = 0) uniform atomic_uint node_counter;

float near_plane = 0.1f;
float far_plane = 512.0f;

uint PackColor(vec4 inColor) 
{
    // 1. 钳位颜色分量到 [0.0, 1.0]，避免超出范围
    vec4 clamped = clamp(inColor, 0.0f, 1.0f);
    
    // 2. 乘以255后加0.5，实现四舍五入（而非截断），再转换为无符号整数
    uvec4 colorUInt4 = uvec4(clamped * 255.0f + 0.5f);
    
    // 3. 按 RGBA 顺序打包到 32 位 uint 中
    return colorUInt4.r | (colorUInt4.g << 8) | (colorUInt4.b << 16) | (colorUInt4.a << 24);
}

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}


void main()
{
    uint pixel_count = atomicCounterIncrement(node_counter);

    uint oldStartOffset = imageAtomicExchange(head_index_image, ivec2(gl_FragCoord.xy), pixel_count);

    uvec4 item;
    item.x = oldStartOffset;
    item.y = PackColor(geo_color);

    float linear_depth = LinearizeDepth(gl_FragCoord.z) / far_plane;
    item.z = floatBitsToUint(linear_depth);
    item.w = 0;

    imageStore(list_buffer, int(pixel_count), item);
}
#version 450 core

#define RANDOM_SAMPLE_NUM 64

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D depth_texture;
layout (binding = 2) uniform sampler2D normal_texture;

// 上面两个纹理中的向量都是在世界空间中的，而AO需要转换到view space中进行
uniform mat4 view;

layout (binding = 1) uniform sampler2D random_texture;
uniform vec3 random_samples[RANDOM_SAMPLE_NUM];

uniform uvec2 AO_texture_size;
const uint random_size = 64;

uniform mat4 projection;
uniform float kernel_radius;
uniform float bias;

vec3 DecodePositionFromDepth(vec2 tex)
{
    float depth = texture(depth_texture, tex).r;
    depth = 2.0 * depth - 1.0;
    vec2 xy = 2.0 * tex - 1.0;
    
    float B = projection[3][2];
    float A = projection[2][2];
    float depth_view = -B / (A + depth);

    vec2 xy_scale = vec2(projection[0][0], projection[1][1]);
    vec2 xy_view = -xy * depth_view / xy_scale;
    
    return vec3(xy_view, depth_view);
}

vec3 DecodeNormal(vec2 tex)
{
    vec2 norm = 2.0 * texture(normal_texture, tex).rg - 1.0;
    float z = sqrt(1.0 - dot(norm, norm));
    return vec3(view * vec4(normalize(vec3(norm, z)), 0.0));
}

vec2 DecodeRandom(vec2 tex)
{
    vec2 sample_tex = tex * vec2(AO_texture_size) / vec2(float(random_size));
    return normalize(vec3(texture(random_texture, sample_tex).rg, 0.0)).rg;
}

void main()
{
    vec3 pos = DecodePositionFromDepth(texCoord);
    vec3 normal = DecodeNormal(texCoord);
    vec3 random_base = vec3(DecodeRandom(texCoord), 0.0);

    vec3 tangent = normalize(random_base - normal * dot(normal, random_base));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (uint i = 0; i < RANDOM_SAMPLE_NUM; i++)
    {
        vec3 local_pos = random_samples[i];
        vec3 sample_pos = TBN * local_pos * kernel_radius + pos;
        vec2 sample_tex = (projection * vec4(sample_pos, 1.0) / -sample_pos.z).xy * 0.5 + 0.5;
        float scene_depth = DecodePositionFromDepth(sample_tex).z;

        float range_check = smoothstep(0.0, 1.0, kernel_radius / abs(scene_depth - sample_pos.z));
        occlusion += (scene_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
    }
    occlusion /= float(RANDOM_SAMPLE_NUM);

    occlusion = 1.0 - occlusion;
    FragColor = vec4(vec3(occlusion), 1.0);
}
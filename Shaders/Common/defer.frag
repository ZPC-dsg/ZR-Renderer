#version 450 core

layout (location = 0) out vec4 position_anisotrophy; // RGB : world position   A : anisotrophy
layout (location = 1) out vec4 albedo_specular; // RGB : albedo   A : specular
layout (location = 2) out vec4 normal_metallic_roughness; // RG : world_normal   B : metallic   A : roughness

layout (location = 0) in vec3 world_normal;
layout (location = 1) in vec3 world_tangent;
layout (location = 2) in vec3 world_position;
layout (location = 3) in vec2 texCoord;

layout (binding = 0) uniform sampler2D diffuse_tex;
layout (binding = 1) uniform sampler2D specular_tex;
layout (binding = 2) uniform sampler2D normal_tex;

uniform float metallic;
uniform float roughness;

vec3 Pow3(vec3 color, float expo)
{
    return vec3(pow(color.r, expo), pow(color.g, expo), pow(color.b, expo));
}

void main()
{
    vec3 albedo_color = texture(diffuse_tex, texCoord).rgb;
    float specular_color = texture(specular_tex, texCoord).r;
    vec3 local_normal = normalize(2.0 * texture(normal_tex, texCoord).rgb - 1.0);

    vec3 albedo_linear = Pow3(albedo_color, 2.2); // 简单伽马矫正，实际上伽马矫正是一个分段函数，这里简单地使用2.2次方

    vec3 world_bitangent = normalize(cross(world_normal, world_tangent));
    mat3 TBN = mat3(world_tangent, world_bitangent, world_normal);
    vec3 real_normal = normalize(TBN * local_normal);

    position_anisotrophy = vec4(world_position, 0.0); // 暂时不考虑各向异性
    albedo_specular = vec4(albedo_linear, specular_color);
    normal_metallic_roughness = vec4(real_normal.xy, metallic, roughness);
}
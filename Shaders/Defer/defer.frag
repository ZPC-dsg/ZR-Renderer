#version 450 core

// 简单地使用lambert光照模型进行渲染

layout (location = 0) out vec4 diff_spec; // RGB : diffuse   A : specular
layout(location = 1) out vec4 norm_specstren; // RG : normal   B : specular_strength   A : shininess

layout (location = 0) in vec3 world_normal;
layout (location = 1) in vec3 world_tangent;
layout (location = 2) in vec2 texCoord;

layout (binding = 0) uniform sampler2D diffuse_tex;
layout (binding = 1) uniform sampler2D specular_tex;
layout (binding = 2) uniform sampler2D normal_tex;

uniform float specular_strength;
uniform float shininess;

vec3 Pow3(vec3 color, float expo)
{
    return vec3(pow(color.r, expo), pow(color.g, expo), pow(color.b, expo));
}

void main()
{
    vec3 diffuse_color = texture(diffuse_tex, texCoord).rgb;
    float specular_color = texture(specular_tex, texCoord).r;
    vec3 local_normal = normalize(2.0 * texture(normal_tex, texCoord).rgb - 1.0);

    vec3 diffuse_linear = Pow3(diffuse_color, 2.2); // 简单伽马矫正，实际上伽马矫正是一个分段函数，这里简单地使用2.2次方

    vec3 world_bitangent = normalize(cross(world_normal, world_tangent));
    mat3 TBN = mat3(world_tangent, world_bitangent, world_normal);
    vec3 real_tangent = normalize(TBN * local_normal);

    diff_spec = vec4(diffuse_linear, specular_color);
    norm_specstren = vec4(real_tangent.xy, specular_strength, shininess);
}
#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexcoord;

layout (location = 0) out vec3 world_normal;
layout (location = 1) out vec3 world_tangent;
layout (location = 2) out vec3 world_position;
layout (location = 3) out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    world_normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    world_tangent = normalize(mat3(transpose(inverse(model))) * aTangent);
    vec4 world_pos = model * vec4(aPosition, 1.0);
    world_position = world_pos.xyz;
    texCoord = aTexcoord;

    gl_Position = projection * view * world_pos;
}
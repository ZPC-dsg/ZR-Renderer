#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 aTransform;
// 注意：mat4会占据4个槽位
layout (location = 5) in vec4 aColor;

layout (location = 0) out vec4 geo_color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * aTransform * vec4(aPos, 1.0);

    geo_color = aColor;
}
#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

invariant gl_Position;

void main()
{
    vec4 world_pos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * world_pos;
}
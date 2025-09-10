#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexcoord;

layout (location = 0) out vec3 view_normal;
layout (location = 1) out vec3 view_tangent;
layout (location = 2) out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    view_normal = mat3(view * model) * aNormal;
    view_tangent = mat3(view * model) * aTangent;
    texCoord = aTexcoord;

    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
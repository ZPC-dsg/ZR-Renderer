#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main() {
    TexCoord = aTexcoord;
    WorldPos = vec3(model * vec4(aPosition, 1.0));
    Normal = normalMat * aNormal;

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
#version 450

out vec4 color;
in vec2 texCoord;
in vec3 normal;

struct ColorBlock {
    vec3 colors;
    float intensity[3];
};

uniform ColorBlock block;
uniform float time;

void main ()
{
    color = vec4(vec3(block.colors.r * block.intensity[0], block.colors.g * block.intensity[1], 
    block.colors.b * block.intensity[2]) * time * normal * vec3(texCoord, 1.0), 1.0);
}
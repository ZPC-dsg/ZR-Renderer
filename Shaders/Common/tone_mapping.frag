#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D bloom_image;

uniform float exposure;

void main()
{
    vec3 before_color = texture(hdr_image, texCoord).rgb;
    vec3 bloom_color = texture(bloom_image, texCoord).rgb;
    
    vec3 color = before_color + bloom_color;
    color = vec3(1.0) - exp(-exposure * color);

    FragColor = vec4(color, 1.0);
}
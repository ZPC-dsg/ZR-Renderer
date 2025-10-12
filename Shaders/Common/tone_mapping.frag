#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D hdr_image;

// Simple Reinhard tone mapping
void main()
{
    vec3 color = texture(hdr_image, texCoord).rgb;
    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
}
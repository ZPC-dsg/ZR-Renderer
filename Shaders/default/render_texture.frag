#version 450 core 

out vec4 FragColor;

in vec2 texCoord;

layout (binding = 0) uniform sampler2D in_texture;

void main()
{
    FragColor = vec4(texture(in_texture, texCoord).rgb, 1.0);
}
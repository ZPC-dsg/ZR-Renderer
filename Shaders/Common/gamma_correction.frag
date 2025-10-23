#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D input_texture;

void main()
{
    vec3 color = texture(input_texture, texCoord).rgb;
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
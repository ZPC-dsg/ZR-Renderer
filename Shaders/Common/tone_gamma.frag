#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D linear_color;

void main()
{
    vec3 color = texture(linear_color, texCoord).rgb;
    // Reinhard tone mapping
    color /= (color + 1.0);
    // Gamma correction;
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
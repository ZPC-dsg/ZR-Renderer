#version 450 core

out vec4 FragColor;

in vec4 color;

void main()
{
    FragColor = vec4(color.rgb, 1.0);
}
#version 450 core

out vec4 FragColor;

in vec2 texCoord;

layout (binding = 0) uniform sampler2D in_depth;

uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depth = texture(in_depth, texCoord).r;

    FragColor = vec4(vec3(LinearizeDepth(depth) / far_plane), 1.0);
}
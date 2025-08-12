#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 world_pos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

const float specular_amount = 64.0;

uniform vec3 camera_pos;

vec3 LightFunction(vec3 light_color, vec3 light_dir) {
    light_dir = normalize(light_dir);
    float diff = dot(light_dir, normalize(Normal));
    vec3 diffuse_color = light_color * diff * texture2D(texture_diffuse1, TexCoords).rgb;

    vec3 eye_dir = normalize(camera_pos - world_pos);
    vec3 half_vec = normalize(eye_dir + light_dir);
    float spec = pow(dot(half_vec, Normal), specular_amount);
    vec3 specular_color = light_color * texture2D(texture_specular1, TexCoords).rgb * spec;

    return diffuse_color + specular_color;
}

void main()
{    
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    ambient = ambient * texture2D(texture_diffuse1, TexCoords).rgb;

    FragColor = vec4(LightFunction(vec3(0.5, 0.0, 0.0), vec3(0.5, -0.3, 0.6)) + LightFunction(vec3(0.0, 0.8, 0.0), 
    vec3(-0.3, 0.4, 0.7)) + LightFunction(vec3(0.0, 0.0, 0.4), vec3(0.6, -0.2, 0.1) + 3.0 * LightFunction(vec3(1.0 ,1.0, 1.0),
    vec3(0.3, -0.3, 0.3))), 1.0);
}
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 col = texture(screenTexture, TexCoords).rgba;
    FragColor = vec4(vec3(col.a), 1.0f);
} 
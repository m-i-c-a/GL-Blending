#version 450 core

layout (location = 0) out vec4 fragColor;

uniform vec3 u_color;
uniform float u_alpha;

void main()
{
    fragColor = vec4(u_color, u_alpha);
}
#version 450 core

layout (location = 0) in vec3 a_pos;

uniform vec3 u_translation;

void main()
{
    gl_Position = vec4(a_pos + u_translation, 1.0f);
}
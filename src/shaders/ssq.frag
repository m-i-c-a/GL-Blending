#version 450 core

in vec2 uv;

out vec4 fragColor;

uniform sampler2D tex;

void main()
{
   fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);// vec4(texture(tex, uv).xyz, 1.0f);
   fragColor = vec4(uv, 0.0f, 1.0f);
}
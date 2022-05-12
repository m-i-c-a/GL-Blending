#version 450 core

out vec2 uv;
 
// void main()
// {
//     float x = -1.0 + float((gl_VertexID & 1) << 2);
//     float y = -1.0 + float((gl_VertexID & 2) << 1);
//     uv.x = 0.5; // (x+1.0)*0.5;
//     uv.y = y; // (y+1.0)*0.5;
//     gl_Position = vec4(x, y, 0, 1);
// }

void main() 
{
    uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);
}

#version 410 core

in vec3 pos;
in vec2 inUv;
out vec2 outUv;

void main() {
    //    vec2 uv = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
    //    gl_Position = vec4(uv * 4 - 1, 0, 1);
    outUv = inUv;
    gl_Position = vec4(pos, 0.0);
}
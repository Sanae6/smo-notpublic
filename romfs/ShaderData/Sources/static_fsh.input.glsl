#version 410 core

precision highp float;
precision highp sampler2D;

const float e = 2.7182818284590452353602874713527;

vec4 noise(vec2 texCoord, float time) {
    float G = e + (time * 0.1);
    vec2 r = (G * sin(G * texCoord.xy));
    return vec4(fract(r.x * r.y * (1.0 + texCoord.x)));
}

uniform float time;
in vec2 fragCoord;
out vec4 color;

void main() {
    color = noise(fragCoord, time);
}

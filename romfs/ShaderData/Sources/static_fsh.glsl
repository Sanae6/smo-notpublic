#version 430
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(binding=0) uniform PGlobals {
	float time;
};
layout(location = 0) out vec4 SV_Target0;
vec2 u_xlat0;
vec2 u_xlat1;
void main()
{
    u_xlat0.x = time * 0.100000001 + 2.71828175;
    u_xlat1.xy = u_xlat0.xx * gl_FragCoord.xy;
    u_xlat1.xy = sin(u_xlat1.xy);
    u_xlat0.xy = u_xlat0.xx * u_xlat1.xy;
    u_xlat0.x = u_xlat0.y * u_xlat0.x;
    u_xlat1.x = gl_FragCoord.x + 1.0;
    u_xlat0.x = u_xlat1.x * u_xlat0.x;
    SV_Target0 = fract(u_xlat0.xxxx);
    return;
}

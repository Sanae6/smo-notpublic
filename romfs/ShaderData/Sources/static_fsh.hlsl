uniform float time;

static float4 gl_FragCoord;
static float4 color;
static float2 fragCoord;

struct SPIRV_Cross_Input
{
  float2 fragCoord : TEXCOORD0;
  float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
  float4 color : SV_Target0;
};

float4 _noise(float2 texCoord, float time_1)
{
  float G = 2.71828174591064453125f + (time_1 * 0.100000001490116119384765625f);
  float2 r = sin(texCoord * G) * G;
  return frac((r.x * r.y) * (1.0f + texCoord.x)).xxxx;
}

void frag_main()
{
  float2 fragCoord_1 = float2(gl_FragCoord.xy);
  float2 param = fragCoord_1;
  float param_1 = time;
  color = _noise(param, param_1);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
  gl_FragCoord = stage_input.gl_FragCoord;
  gl_FragCoord.w = 1.0 / gl_FragCoord.w;
  fragCoord = stage_input.fragCoord;
  frag_main();
  SPIRV_Cross_Output stage_output;
  stage_output.color = color;
  return stage_output;
}

:name "Fluid FS - Curl step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidVelocity1

layout (location = 0) out vec4 Curl;

SHADER
{
  float l = texture(gFluidVelocity1, vLeftCoord).y;
  float r = texture(gFluidVelocity1, vRightCoord).y;
  float t = texture(gFluidVelocity1, vTopCoord).x;
  float b = texture(gFluidVelocity1, vBottomCoord).x;
  float vorticity = r - l - t + b;
  Curl = vec4(0.5 * vorticity, 0.0, 0.0, 1.0);
  //Curl = vec4(-128*(vTopCoord - vBottomCoord), 0, 1);
  //Curl = vec4(-128*(vRightCoord - vLeftCoord), 0, 1);
}

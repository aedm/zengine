:name "Fluid FS - Divergence step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidVelocity2

layout (location = 0) out vec4 Divergence;

SHADER
{
  vec2 c = texture2D(gFluidVelocity2, vTexCoord).xy;
  float l = vLeftCoord.x < 0 ? -c.x : texture(gFluidVelocity2, vLeftCoord).x;
  float r = vRightCoord.x > 1 ? -c.x : texture(gFluidVelocity2, vRightCoord).x;
  float t = vTopCoord.y > 1 ? -c.y : texture(gFluidVelocity2, vTopCoord).y;
  float b = vBottomCoord.y < 0 ? -c.y : texture(gFluidVelocity2, vBottomCoord).y;

  float div = 0.5 * (r - l + t - b);
  Divergence = vec4(div, 0.0, 0.0, 1.0);
}

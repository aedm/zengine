:name "Fluid FS - Gradient subtraction step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidPressure
:global sampler2D gFluidVelocity2

layout (location = 0) out vec4 Velocity3;

SHADER
{
  float l = texture(gFluidPressure, vLeftCoord).x;
  float r = texture(gFluidPressure, vRightCoord).x;
  float t = texture(gFluidPressure, vTopCoord).x;
  float b = texture(gFluidPressure, vBottomCoord).x;
  vec2 velocity = texture(gFluidVelocity2, vTexCoord).xy;
  velocity.xy -= vec2(r - l, t - b);
  Velocity3 = vec4(velocity, 0, 1);
}

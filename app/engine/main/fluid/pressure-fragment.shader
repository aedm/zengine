:name "Fluid FS - Pressure step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidPressure
:global sampler2D gFluidDivergence

layout (location = 0) out vec4 PressureOutput;

SHADER
{
  float l = texture(gFluidPressure, vLeftCoord).x;
  float r = texture(gFluidPressure, vRightCoord).x;
  float t = texture(gFluidPressure, vTopCoord).x;
  float b = texture(gFluidPressure, vBottomCoord).x;

  float c = texture(gFluidPressure, vTexCoord).x;
  float divergence = texture(gFluidDivergence, vTexCoord).x;

  float pressure = (l + r + b + t - divergence) * 0.25;
  PressureOutput = vec4(pressure, 0.0, 0.0, 1.0);
}

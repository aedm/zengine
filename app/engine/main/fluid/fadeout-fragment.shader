:name "Fluid FS - Pressure fade step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidPressure
:global float gFluidPressureFade

layout (location = 0) out vec4 PressureOutput;

SHADER
{
  PressureOutput = texture(gFluidPressure, vTexCoord) * gFluidPressureFade;
}

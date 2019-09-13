:name "Fluid FS - Advection step"
:returns void

:input vec2 vTexCoord

:global sampler2D gFluidVelocity3
:global sampler2D gFluidColor
:global vec2 gRenderTargetSizeRecip
:global float gFluidDeltaTime
:global float gFluidDissipation
:global sampler2D gFluidVelocity1
:global sampler2D gFluidVelocity2

layout (location = 0) out vec4 Output;

SHADER
{
  vec2 coord = vTexCoord - texture(gFluidVelocity1, vTexCoord).xy * gRenderTargetSizeRecip * gFluidDeltaTime;
  vec4 result = texture(gFluidColor, coord);
  float decay = 1.0 + gFluidDissipation * gFluidDeltaTime;
  Output = result / decay;
}

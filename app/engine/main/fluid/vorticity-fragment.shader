:name "Fluid FS - Vorticity step"
:returns void

:input vec2 vTexCoord
:input vec2 vLeftCoord
:input vec2 vRightCoord
:input vec2 vTopCoord
:input vec2 vBottomCoord

:global sampler2D gFluidVelocity1
:global sampler2D gFluidCurl
:global float gFluidCurlAmount
:global float gFluidForceDamp
:global float gFluidDeltaTime

layout (location = 0) out vec4 Velocity2;

SHADER
{
  float l = texture(gFluidCurl, vLeftCoord).x;
  float r = texture(gFluidCurl, vRightCoord).x;
  float t = texture(gFluidCurl, vTopCoord).x;
  float b = texture(gFluidCurl, vBottomCoord).x;
  float c = texture(gFluidCurl, vTexCoord).x;

  vec2 force = vec2(abs(t) - abs(b), abs(l) - abs(r)) * 0.5;
  force /= length(force) + gFluidForceDamp;
  force *= gFluidCurlAmount * c;
  //force.y *= -1.0;
  vec2 vel = texture(gFluidVelocity1, vTexCoord).xy;
  Velocity2 = vec4(vel + force * gFluidDeltaTime, 0.0, 1.0);
}

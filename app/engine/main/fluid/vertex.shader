:name "Fluid vertex shader"

:input vec3 aPosition;
:input vec2 aTexCoord;

:output vec2 vTexCoord
:output vec2 vLeftCoord
:output vec2 vRightCoord
:output vec2 vTopCoord
:output vec2 vBottomCoord

:global vec2 gRenderTargetSizeRecip

SHADER
{
  vec3 halfTexel = vec3(gRenderTargetSizeRecip, 0);

  gl_Position = vec4(aPosition, 1.0); 
  vTexCoord = aTexCoord;
  vLeftCoord = aTexCoord - halfTexel.xz;
  vRightCoord = aTexCoord + halfTexel.xz;
  vTopCoord = aTexCoord + halfTexel.zy;
  vBottomCoord = aTexCoord - halfTexel.zy;
}

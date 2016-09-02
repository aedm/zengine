:name "PostProcess gaussian blur, vertical pass"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global sampler2D gGBufferSourceA
:global vec2 gRenderTargetSizeRecip
:output vec4 FragColor


const int kernelSize = 5;
float weight[kernelSize*2+1] = 
  float[] (0.0093, 0.028002, 0.065984, 0.121703, 0.175713, 0.198596, 0.175713, 0.121703, 0.065984, 0.028002, 0.0093);


SHADER
{             
  vec3 result = vec3(0.0, 0.0, 0.0);
  float d = -kernelSize * gRenderTargetSizeRecip.y;
  for(int i = 0; i < kernelSize * 2 + 1; ++i)
  {
    result += texture2D(gPPGauss, vTexCoord + vec2(0.0, d)).rgb * weight[i];
    d += gRenderTargetSizeRecip.y;
  }        
  FragColor = vec4(result, 1.0) *0.8+ texture(gGBufferSourceA, vTexCoord);
}

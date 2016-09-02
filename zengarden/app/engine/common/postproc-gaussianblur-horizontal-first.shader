:name "PostProcess gaussian blur, horizontal pass"

:input vec2 vTexCoord
:global sampler2D gGBufferSourceA
:global vec2 gRenderTargetSizeRecip
:output vec4 FragColor


const int kernelSize = 5;
float weight[kernelSize*2+1] = 
  float[] (0.0093, 0.028002, 0.065984, 0.121703, 0.175713, 0.198596, 0.175713, 0.121703, 0.065984, 0.028002, 0.0093);


SHADER
{             
  vec3 result = vec3(0.0, 0.0, 0.0);
  float d = -kernelSize * gRenderTargetSizeRecip.x;
  for(int i = 0; i < kernelSize * 2 + 1; ++i)
  {
    result += max(texture(gGBufferSourceA, vTexCoord + vec2(0.0, d)).rgb - vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0)) * weight[i];
    //result += texture2D(gGBufferSourceA, vTexCoord + vec2(d, 0.0)).rgb * weight[i];
    d += gRenderTargetSizeRecip.x;
  }        
  FragColor = vec4(result, 1.0);
}

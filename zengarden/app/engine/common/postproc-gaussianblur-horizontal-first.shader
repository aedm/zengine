:name "PostProcess gaussian blur, horizontal pass"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global vec2 gPPGaussPixelSize
:global float gPPGaussRelativeSize
:output vec4 FragColor


const int kernelSize = 5;
float weight[kernelSize*2+1] = float[] (0.066414, 0.079465, 0.091364, 0.100939, 0.107159, 0.109317, 0.107159, 0.100939, 0.091364, 0.079465, 0.066414);


SHADER
{             
  vec3 result = vec3(0.0, 0.0, 0.0);
  vec2 d = vTexCoord * gPPGaussRelativeSize - vec2(gPPGaussPixelSize.x * kernelSize, 0.0);
  for(int i = 0; i < kernelSize * 2 + 1; ++i)
  {
    result += clamp(texture(gPPGauss, d).rgb - 1.0, 0.0, 1.5) * weight[i];
    d.x += gPPGaussPixelSize.x;
  }        
  FragColor = vec4(result, 1.0);
}

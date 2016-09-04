:name "PostProcess gaussian blur, vertical pass"

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
  vec2 d = vTexCoord * gPPGaussRelativeSize - vec2(0.0, gPPGaussPixelSize.y * kernelSize);
  for(int i = 0; i < kernelSize * 2 + 1; ++i)
  {
    result += texture2D(gPPGauss, d).rgb * weight[i];
    d.y += gPPGaussPixelSize.y;
  }        
  FragColor = vec4(result, 1.0);
}

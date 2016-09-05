:name "PostProcess gaussian blur, horizontal pass"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global vec2 gPPGaussPixelSize
:global vec2 gPPGaussRelativeSize
:output vec4 FragColor


const int kernelSize = 10;
float weight[kernelSize*2+1] = float[] (0.011254, 0.016436, 0.023066, 0.031105, 0.040306, 0.050187, 0.060049, 0.069041, 0.076276, 0.080977, 0.082607, 0.080977, 0.076276, 0.069041, 0.060049, 0.050187, 0.040306, 0.031105, 0.023066, 0.016436, 0.011254);


SHADER
{             
  vec3 result = vec3(0.0, 0.0, 0.0);
  vec2 d = vTexCoord * gPPGaussRelativeSize - vec2(gPPGaussPixelSize.x * kernelSize, 0.0);
  for(int i = 0; i < kernelSize * 2 + 1; ++i)
  {
    result += texture2D(gPPGauss, d).rgb * weight[i];
    d.x += gPPGaussPixelSize.x;
  }        
  FragColor = vec4(result, 1.0);
}

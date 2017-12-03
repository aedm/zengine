:name "PostProcess gaussian blur, blending"

:input vec2 vTexCoord

:global sampler2D gPPGauss
:global sampler2DMS gGBufferSourceA
:global float gGBufferSampleCount
:global vec2 gPPGaussPixelSize
:global vec2 gPPGaussRelativeSize
:global float gTime

:output vec4 FragColor


SHADER
{             
  vec3 bloom = texture2D(gPPGauss, vTexCoord * gPPGaussRelativeSize).rgb;

  vec3 pixel = vec3(0.0, 0.0, 0.0);
  ivec2 coord = ivec2(textureSize(gGBufferSourceA) * vTexCoord);
  
  // Intel doesn't fckn support "textureSamples", which is OpenGL 4.5 
  // int sampleCount = textureSamples(gGBufferSourceA);
  int sampleCount = int(gGBufferSampleCount);
  
  for (int i = 0; i < sampleCount; i++) {
    pixel += clamp(texelFetch(gGBufferSourceA, coord, i).rgb, 0, 1);
  }
  
  vec4 color = vec4(bloom + pixel / float(sampleCount), 1.0);
  float dither = (Noise(vTexCoord) - 0.5) / 256.0;
  FragColor = color + dither;
}

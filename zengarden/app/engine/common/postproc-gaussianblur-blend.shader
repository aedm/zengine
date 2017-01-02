:name "PostProcess gaussian blur, blending"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global sampler2DMS gGBufferSourceA
:global float gGBufferSampleCount
:global vec2 gPPGaussPixelSize
:global vec2 gPPGaussRelativeSize
:output vec4 FragColor


SHADER
{             
  vec3 bloomPixel = texture2D(gPPGauss, vTexCoord * gPPGaussRelativeSize).rgb;
  
  vec3 pixel = vec3(0.0, 0.0, 0.0);
  ivec2 coord = ivec2(textureSize(gGBufferSourceA) * vTexCoord);
  
  // Intel doesn't fckn support "textureSamples", which is OpenGL 4.5 
  // int sampleCount = textureSamples(gGBufferSourceA);
  int sampleCount = int(gGBufferSampleCount);
  
  for (int i = 0; i < sampleCount; i++) {
    pixel += clamp(texelFetch(gGBufferSourceA, coord, i).rgb, 0, 1);
  }
  
  vec3 bPixel = clamp(bloomPixel, 0.0, 1.0);
  
  float powfac = 1.0;
  vec3 bloom = vec3(pow(bPixel.r, powfac), pow(bPixel.g, powfac), pow(bPixel.b, powfac));
  FragColor = vec4(bloom * 1.0, 0.0) + vec4(pixel / float(sampleCount), 1.0);
  
//  FragColor = vec4(pow(bloomPixel, 0.5), 0.0) * 40.0 + vec4(pixel / float(sampleCount), 1.0);
}

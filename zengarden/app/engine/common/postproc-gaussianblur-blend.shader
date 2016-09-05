:name "PostProcess gaussian blur, blending"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global sampler2DMS gGBufferSourceA
:global vec2 gPPGaussPixelSize
:global vec2 gPPGaussRelativeSize
:output vec4 FragColor


SHADER
{             
  vec3 bloomPixel = texture2D(gPPGauss, vTexCoord * gPPGaussRelativeSize).rgb;
  
  vec3 pixel = vec3(0.0, 0.0, 0.0);
  ivec2 coord = ivec2(textureSize(gGBufferSourceA) * vTexCoord);
  int sampleCount = textureSamples(gGBufferSourceA);
  for (int i = 0; i < sampleCount; i++) {
    pixel += clamp(texelFetch(gGBufferSourceA, coord, i).rgb, 0, 1);
  }
  
  float powfac = 1.0;
  vec3 bloom = vec3(pow(bloomPixel.r, powfac), pow(bloomPixel.g, powfac), pow(bloomPixel.b, powfac));
  FragColor = vec4(bloom * 2.0, 0.0) + vec4(pixel / float(sampleCount), 1.0);
//  FragColor = vec4(pow(bloomPixel, 0.5), 0.0) * 40.0 + vec4(pixel / float(sampleCount), 1.0);
}

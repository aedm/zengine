:name "PostProcess gaussian blur, blending"

:input vec2 vTexCoord
:global sampler2D gPPGauss
:global sampler2DMS gGBufferSourceA
:global vec2 gPPGaussPixelSize
:global float gPPGaussRelativeSize
:output vec4 FragColor


const int kernelSize = 50;
float weight[kernelSize*2+1] = float[] (0.005757, 0.005937, 0.00612, 0.006305, 0.00649, 0.006678, 0.006866, 0.007055, 0.007245, 0.007436, 0.007626, 0.007817, 0.008007, 0.008197, 0.008386, 0.008574, 0.008761, 0.008946, 0.00913, 0.009312, 0.009491, 0.009667, 0.009841, 0.010012, 0.010179, 0.010342, 0.010502, 0.010657, 0.010808, 0.010954, 0.011096, 0.011232, 0.011362, 0.011487, 0.011606, 0.011719, 0.011826, 0.011926, 0.01202, 0.012106, 0.012186, 0.012259, 0.012324, 0.012382, 0.012432, 0.012475, 0.01251, 0.012538, 0.012557, 0.012569, 0.012573, 0.012569, 0.012557, 0.012538, 0.01251, 0.012475, 0.012432, 0.012382, 0.012324, 0.012259, 0.012186, 0.012106, 0.01202, 0.011926, 0.011826, 0.011719, 0.011606, 0.011487, 0.011362, 0.011232, 0.011096, 0.010954, 0.010808, 0.010657, 0.010502, 0.010342, 0.010179, 0.010012, 0.009841, 0.009667, 0.009491, 0.009312, 0.00913, 0.008946, 0.008761, 0.008574, 0.008386, 0.008197, 0.008007, 0.007817, 0.007626, 0.007436, 0.007245, 0.007055, 0.006866, 0.006678, 0.00649, 0.006305, 0.00612, 0.005937, 0.005757);


SHADER
{             
  vec3 bloomPixel = texture2D(gPPGauss, vTexCoord * gPPGaussRelativeSize).rgb;
  
  vec3 pixel = vec3(0.0, 0.0, 0.0);
  ivec2 coord = ivec2(textureSize(gGBufferSourceA) * vTexCoord);
  int sampleCount = textureSamples(gGBufferSourceA);
  for (int i = 0; i < sampleCount; i++) {
    pixel += clamp(texelFetch(gGBufferSourceA, coord, i).rgb, 0, 1);
  }
  
  FragColor = vec4(bloomPixel, 0.0) * 1.0 + vec4(pixel / float(sampleCount), 1.0);
}
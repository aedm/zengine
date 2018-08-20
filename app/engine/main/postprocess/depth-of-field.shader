:name "Depth of field, fragment shader"

:input vec2 vTexCoord

:global sampler2DMS gGBufferSourceA
:global sampler2DMS gDepthBufferSource
:global float gGBufferSampleCount
:global float gPPDofFocusDistance
:global float gPPDofBlur
:global mat4 gProjection
:global float gTime

:output vec4 FragColor

vec3 pal( in float t)
{
    return vec3(0.5) + vec3(0.5)*cos( 6.28318*(vec3(3.2, 2.324, 1.3232)*t*0.1 + vec3(0)) );
}

SHADER
{ 
  vec2 gbufferSize = textureSize(gGBufferSourceA);           
  int gbufferSampleCount = int(gGBufferSampleCount);
  
  // Random rotation
  float rand = Noise(gl_FragCoord.xy + vec2(1.2341234, 10.934367 + gl_SampleID)) * gTime * 100.1 + gl_SampleID;
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);

  vec4 zProject = (vec4(0, 0, -gPPDofFocusDistance, 1) * gProjection);
  float focusDepth = zProject.z / zProject.w;

  vec2 blurUVCorrect = vec2(gPPDofBlur * 0.1) / gbufferSize;
  
	float referenceDepth = texelFetch(gDepthBufferSource, ivec2(gbufferSize * vTexCoord), gl_SampleID).z;
  const float depthTolerance = 0.001;
  float maxDepth = min(referenceDepth + depthTolerance, 1.0);  
  
  vec3 sumColor = vec3(0, 0, 0);
  float sumAlpha = 0;
  
  int sampleCount = min(poissonCount, 100);
  for (int poissonIndex = 0; poissonIndex < sampleCount; poissonIndex++) {
    vec3 poissonPoint = poissonDisk[poissonIndex];

    vec2 pVec = poissonPoint.xy * rotrand;
    vec2 uv = vTexCoord + pVec * blurUVCorrect;
    
    if (uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1) continue;
    
    ivec2 coord = ivec2(gbufferSize * uv);
  
    float sampleDepth = texelFetch(gDepthBufferSource, coord, gl_SampleID).z;
    if (sampleDepth >= maxDepth) continue;

    const float p = 0.2;
    float coc = min(abs(sampleDepth - focusDepth) * 100, 1.0);
    float cocAlpha = coc - poissonPoint.z * 0.08;
    
    if (cocAlpha <= 0.0) continue;
    float zAlpha = max((maxDepth - sampleDepth) / depthTolerance, 0);
    float alpha = cocAlpha * zAlpha;
        
    vec3 c = texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;
    
    sumColor += c * alpha;
    sumAlpha += alpha;
  }
  
  FragColor = vec4((sumAlpha > 0) ? (sumColor / sumAlpha) : vec3(0), 1);
}

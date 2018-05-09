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

#define DOF_LEVELS 8

//float 

SHADER
{ 
  // Sum of color and number of samples for each level
  const int levelCount = DOF_LEVELS * 2 + 1;
  vec3 levelColors[levelCount];
  int levelSampleCount[levelCount];
  for (int i=0; i<levelCount; i++) {
	levelColors[i] = vec3(0, 0, 0);
	levelSampleCount[i] = 0;
  }
  
  vec2 gbufferSize = textureSize(gGBufferSourceA);           
  int gbufferSampleCount = int(gGBufferSampleCount);
  
  // Random rotation
  float rand = Noise(gl_FragCoord.xy + vec2(10.2341234, 0.934367) * gl_FragCoord.z) * 100 + gTime;
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);

  //float fragmentDepth = texelFetch(gDepthBufferSource, ivec2(gbufferSize * vTexCoord), gl_SampleID).z;
  
  vec4 zProject = (vec4(0, 0, -gPPDofFocusDistance, 1) * gProjection);
  float focusDepth = zProject.z / zProject.w;
  
  //vec3 color = vec3(0, 0, 0);
  //float sampleCount = 0;

  //float blurFactor = abs(focusDepth - fragmentDepth) * gPPDofBlur * 10;
  vec2 blurUVCorrect = vec2(gPPDofBlur) / gbufferSize;
  
  int pointCount = min(100, poissonCount);
  for (int poissonIndex = 0; poissonIndex < pointCount; poissonIndex++) {
    vec3 poissonPoint = poissonDisk[poissonIndex];
	vec2 pVec = poissonPoint.xy * rotrand;
    vec2 uv = vTexCoord + pVec * blurUVCorrect;
    ivec2 coord = ivec2(gbufferSize * uv);
    
	float sampleDepth = texelFetch(gDepthBufferSource, coord, gl_SampleID).z;

	// Calculate distance level
	float dist = focusDepth - sampleDepth;
	float absDist = abs(dist);
	float tolerance = absDist * 100.0;
	if (poissonPoint.z > tolerance) continue;

	int level = int(log(1 + absDist * 100));
	level = min(level, DOF_LEVELS);
	level = focusDepth > sampleDepth ? DOF_LEVELS + level : DOF_LEVELS - level;
	
	vec4 sampleColor = texelFetch(gGBufferSourceA, coord, gl_SampleID);
    levelColors[level] += sampleColor.rgb;
	levelSampleCount[level]++;
	
	//if (sampleDepth >= 1.0) continue;
	//color += texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;	
  }
  
  // Blur color
  vec3 color = vec3(0, 0, 0);
  for (int i=DOF_LEVELS; i>=0; i--) {
	if (levelSampleCount[i] == 0) continue;
	color = levelColors[i] / float(levelSampleCount[i]);
	break;
  }
   
  FragColor = vec4(color, 1);
}

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
  vec3 backColors[DOF_LEVELS], frontColors[DOF_LEVELS];
  int backSampleCount[DOF_LEVELS], frontSampleCount[DOF_LEVELS];
  int backSampleTotal[DOF_LEVELS], frontSampleTotal[DOF_LEVELS];
  for (int i=0; i<DOF_LEVELS; i++) {
	backColors[i] = vec3(0, 0, 0);
	frontColors[i] = vec3(0, 0, 0);
	backSampleCount[i] = 0;
	frontSampleCount[i] = 0;
	backSampleTotal[i] = 0;
	frontSampleTotal[i] = 0;
  }
  
  vec2 gbufferSize = textureSize(gGBufferSourceA);           
  int gbufferSampleCount = int(gGBufferSampleCount);
  
  // Random rotation
  float rand = Noise(gl_FragCoord.xy + vec2(10.2341234, 0.934367) * gl_FragCoord.z) * 1 + gTime*0.01;
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);

  vec4 zProject = (vec4(0, 0, -gPPDofFocusDistance, 1) * gProjection);
  float focusDepth = zProject.z / zProject.w;

  //float blurFactor = abs(focusDepth - fragmentDepth) * gPPDofBlur * 10;
  vec2 blurUVCorrect = vec2(gPPDofBlur) / gbufferSize;
  
  const float K = 0.02 * gPPDofBlur;
  const float J = 16.1;
  const float d0 = J / 2;
  bool hasInFocus = false;
  vec3 focusColor; 

  vec3 color = vec3(0, 0, 0);

  int pointCount = min(40, poissonCount);
  for (int poissonIndex = 0; poissonIndex < pointCount; poissonIndex++) {
    vec3 poissonPoint = poissonDisk[poissonIndex];
	vec2 pVec = poissonPoint.xy * rotrand;
    vec2 uv = vTexCoord + pVec * blurUVCorrect;
    ivec2 coord = ivec2(gbufferSize * uv);
    
	float sampleDepth = texelFetch(gDepthBufferSource, coord, gl_SampleID).z;
	
	float d = sampleDepth - focusDepth;
	int l = clamp(int(abs(d) * J), -DOF_LEVELS+1, DOF_LEVELS-1);

	color = vec3(0.5 + l * 0.1, 0, 0);
	
	// Detect point in focus
	if (l == 0) {
	  if (poissonIndex > 0) continue;
	  // in focus
	  hasInFocus = true;
  	  focusColor = texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;
	  continue;
	}

	// Calculate distance level	
	if (sampleDepth < focusDepth) {
	  // in front of focus
	  
	}
	else { 
	  // behind focus
      //if (hasInFocus) continue;
	  //float r = K * (d - d0);
	  
	  //if (r < poissonPoint.z) continue;
	  
	  //float ds = poissonPoint.z / K + d0;
  	  //int ls = clamp(int(abs(ds) * J), 0, DOF_LEVELS-1);
	  //backSampleTotal[ls]++;

	  //backColors[l] += texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;
	  //backSampleCount[l]++;
    }
  }

  if (!hasInFocus) {
    for (int i=1; i<DOF_LEVELS; i++) backSampleTotal[i] += backSampleTotal[i-1];

    // Blur back color
    for (int i=DOF_LEVELS; i>=0; i--) {
	  int bst = backSampleTotal[i];
	  int bsc = backSampleCount[i];
	  if (bst == 0 || bsc == 0) continue;
	  float alpha = float(bsc) / float(bst);
	  color += mix(color, backColors[i] / float(bsc), alpha);
    }
  }
  else {
    color = focusColor;
  }
   
  FragColor = vec4(color, 1);
}

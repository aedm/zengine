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

vec3 pal( in float t)
{
    return vec3(0.5) + vec3(0.5)*cos( 6.28318*(vec3(3.2, 2.324, 1.3232)*t*0.1 + vec3(0)) );
}

SHADER
{ 
  // Sum of color and number of samples for each level
  vec3 backColors[DOF_LEVELS], frontColors[DOF_LEVELS];
  float backSampleCount[DOF_LEVELS], frontSampleCount[DOF_LEVELS];
  float backSampleTotal[DOF_LEVELS], frontSampleTotal[DOF_LEVELS];
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
  float rand = Noise(gl_FragCoord.xy + vec2(1.2341234, 10.934367 + gl_SampleID)) * gTime * 10.1 + gl_SampleID;
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);

  vec4 zProject = (vec4(0, 0, -gPPDofFocusDistance, 1) * gProjection);
  float focusDepth = zProject.z / zProject.w;

  vec2 blurUVCorrect = vec2(gPPDofBlur) / gbufferSize;
  
  const float K = 1000.02 * gPPDofBlur;
  const float J = 500.1;
  const float d0 = 0.5 / J;
  bool hasInFocus = false;
  vec3 focusColor; 

  #if 0
	float sampleDepth = texelFetch(gDepthBufferSource, ivec2(gbufferSize * vTexCoord), gl_SampleID).z;
    float d = sampleDepth - focusDepth;
	float dj = d * J;
	int l = clamp(int(dj), -DOF_LEVELS, DOF_LEVELS);
	FragColor = vec4(pal(float(l)), 1);
	return;
  #endif
  
  vec3 color = vec3(0, 0, 0);

  for (int dofLevel = 2; dofLevel >= 0; dofLevel--) {
    vec3 dofColor = vec3(0);
	int weigth = 0;
	float sumAlpha = 0;
    const float maxRad = float(dofLevel + 1.8) * 1.0;

	for (int poissonIndex = 0; poissonIndex < poissonCount; poissonIndex++) {
	  vec3 poissonPoint = poissonDisk[poissonIndex];
	  if (poissonPoint.z > maxRad) break;

	  vec2 pVec = poissonPoint.xy * rotrand;
	  vec2 uv = vTexCoord + pVec * blurUVCorrect;
	  ivec2 coord = ivec2(gbufferSize * uv);
	
	  float sampleDepth = texelFetch(gDepthBufferSource, coord, gl_SampleID).z;
	  if (sampleDepth >= 1.0) continue;
	  
	  // compute texel level
      float d = sampleDepth - focusDepth;
	  float dj = d * J;
	  int l = clamp(int(dj), -DOF_LEVELS, DOF_LEVELS);

	  if (l > dofLevel+1) continue;
	  if (l < dofLevel-1) continue;
      weigth += 1;

	  
      vec3 c = texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;
	  float alpha = 1.0;
	  
	  if (l == dofLevel-1) {
    	// small tolerance region nearer than focus
	    // -0.5 < dj < 0.0
	    alpha = dj - float(dofLevel) + 1.0;
	  }
	  else if (l == dofLevel+1) {
    	// small tolerance region farther than dof level
	    alpha = float(dofLevel) - dj + 2.0;
	  }
      
      dofColor += c * alpha;
	  sumAlpha += alpha;
	}
	
	if (weigth > 0) {
		dofColor /= sumAlpha;
		sumAlpha /= float(weigth);
			
		// blend
		color = mix(color, dofColor, sumAlpha);
	}
	
	//color = pal(float(sumAlpha));
	//break;
  }
  
  FragColor = vec4(color, 1);
  return;
}

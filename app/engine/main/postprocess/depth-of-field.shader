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

SHADER
{ 
  vec2 gbufferSize = textureSize(gGBufferSourceA);           
  int gbufferSampleCount = int(gGBufferSampleCount);
  
  float rand = Noise(gl_FragCoord.xy + vec2(10.2341234, 0.934367) * gl_FragCoord.z * gl_SampleID) * 100 + gTime;
  float sinrand = sin(rand);
  float cosrand = cos(rand);
  mat2 rotrand = mat2(cosrand, sinrand, sinrand, -cosrand);


  float fragmentDepth = texelFetch(gDepthBufferSource, ivec2(gbufferSize * vTexCoord), gl_SampleID).z;
  
  vec4 zProject = (vec4(0, 0, -gPPDofFocusDistance, 1) * gProjection);
  float focusDepth = zProject.z / zProject.w;

  //FragColor = vec4(1.0 - abs(focusDepth - fragmentDepth)* 400, 0, 0, 1); return;
  
  vec3 color = vec3(0, 0, 0);
  float sampleCount = 0;

  float blurFactor = abs(focusDepth - fragmentDepth) * gPPDofBlur * 10;
  vec2 blurUVCorrect = vec2(blurFactor) / gbufferSize;
  
  int pointCount = min(10, poissonCount);
  for (int poissonIndex = 0; poissonIndex < pointCount; poissonIndex++) {
    vec3 poissonPoint = poissonDisk[poissonIndex];
	vec2 pVec = poissonPoint.xy * rotrand;
    vec2 uv = vTexCoord + pVec * blurUVCorrect;
    ivec2 coord = ivec2(gbufferSize * uv);
    
	  float sampleDepth = texelFetch(gDepthBufferSource, coord, gl_SampleID).z;
	  if (sampleDepth >= 1.0) continue;
	  
	  color += texelFetch(gGBufferSourceA, coord, gl_SampleID).rgb;	
	  
	  //float inFocus = abs(sampleDepth - focusDepth);
	  //float radius = gPPDofBlur * inFocus * 100;
	  //float bokehArea = radius * radius;
	  //if (bokehArea >= radiusSquared) {
	    //float contrib = exp(-1000.1 * inFocus) / gGBufferSampleCount;
		//color += texelFetch(gGBufferSourceA, coord, i).rgb * contrib;	
		//sampleCount += offFocus;	
	  //}
  }
  
  color = vec3((color / (float(pointCount))).rgb);
  
  FragColor = vec4(color.rgb, 1);
}

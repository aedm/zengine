#version 430

:constant 
:define integer IMAGE_SIZE

:global image2DMS gGBufferSourceA
:global image2DMS gDepthBufferSource

:input integer ScaleDown
:input integer GaussKernelSize
:input float GaussKernel[16]

layout(local_size_x = IMAGE_SIZE) in;

SHADER {
	int sampleCount = imageSamples(gDepthBufferSource);
	
	
	imageLoad(gGBufferSourceA)

  // base pixel colour for image
  vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
  // get index in global work group i.e x,y position
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  
  //
  // interesting stuff happens here later
  //
  
  // output to a specific pixel in the image
  imageStore(img_output, pixel_coords, pixel);
}
#version 430

#define MAX_BLUR_RADIUS 8
#define SAMPLE_COUNT 4

:global image2DMS gGBufferSourceA
:global image2DMS gDepthBufferSource

:input int ScaleDown
:input int BlurRadius

const float BlurKernel[MAX_BLUR_RADIUS][MAX_BLUR_RADIUS] = {
  { 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0.45186276187760605, 0.274068619061197, 0, 0, 0, 0, 0, 0 },
  { 0.3036412247131362, 0.2364760235793509, 0.11170336406408092, 0, 0, 0, 0, 0 },
  { 0.2400350638530067, 0.2031852948845218, 0.12323811095021497, 0.05355906223875996, 0, 0, 0, 0 },
  { 0.20416368871516752, 0.18017382291138087, 0.1238315368057753, 0.0662822452863612, 0.027630550638898826, 0, 0, 0 },
  { 0.18078549298024618, 0.16358147868660405, 0.12118413997709437, 0.07350189636701518, 0.03649996153665893, 0.014839776942504379, 0, 0 },
  { 0.16410439771606886, 0.15098333453477822, 0.11758593916641716, 0.07751742863877989, 0.04325744959016075, 0.020433372343272454, 0.008170276868557052, 0 },
  { 0.15144468046442158, 0.14100450516459187, 0.11380723852605518, 0.07962779935118171, 0.04829670167193745, 0.025393827358369248, 0.011574354612399057, 0.004573233083254649 },
};

const int MaxPixelCount = gl_WorkGroupSize.x + 2 * MAX_BLUR_RADIUS;

shared vec4 sourceColor[MaxPixelCount][SAMPLE_COUNT];
shared float sourceDepth[MaxPixelCount][SAMPLE_COUNT];

layout(local_size_x = 256) in;

SHADER {    
  const uint x = gl_GlobalInvocationID.x;
  const uint y = gl_GlobalInvocationID.y;
  
  // Load from memory
  for (uint i=0; i<SAMPLE_COUNT; i++) {
    sourceColor[x][i] = imageLoad(gGBufferSourceA, gl_GlobalInvocationID, i);
  }  
	
  const int totalBlurRadius = BlurRadius * ScaleDown;
  if (x < totalBlurRadius) {
    
  }
  
  // Wait for it
  memoryBarrierShared();
	

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
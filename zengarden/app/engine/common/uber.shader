:name "UberShader"
:returns void

// G-Buffer
// Both read-only textures and write-only buffers are defined, 
// shader source compiler should eliminate unused parts.

// 1. [4x16F] RGB: HDR final color, A: unused
:output vec4 GBufferTargetA
:global sampler2D gGBufferSourceA


// Texcoords of fullscreen quads
:input vec2 vTexCoord


void GBufferWriteFinalColor(vec3 rgb) {
  GBufferTargetA.rgb = rgb;
}

vec3 GBufferReadFinalColor() {
  return texture2D(gGBufferSourceA, vTexCoord).rgb;
}

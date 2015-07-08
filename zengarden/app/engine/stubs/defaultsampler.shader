:name "sampler (def)"
:returns vec4

:param sampler2D Texture
:input vec2 vTexCoord

SHADER
{
  return texture2D(Texture, vTexCoord);
}

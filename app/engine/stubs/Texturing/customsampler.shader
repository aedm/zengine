:name "sampler"
:returns vec4

:param sampler2D Texture
:param vec2 TexCoord

SHADER
{
  return texture2D(Texture, TexCoord);
}

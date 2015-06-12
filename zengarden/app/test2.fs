:name "test"
:returns void

:output vec4 Color
:input vec2 vTexCoord

:param float Green
:param sampler2D Texture 

SHADER
{
	vec4 c = texture2D(Texture, vTexCoord);
	Color = vec4(c.x, Green, vTexCoord.y, 1.0);
		
}

:name "default FS"
:returns void

:output vec4 FragColor
:input vec2 vTexCoord

:param vec4 Color
//:param sampler2D Texture 

SHADER
{
	//vec4 c = texture2D(Texture, vTexCoord);
	//FragColor = vec4(c.x, Green, vTexCoord.y, 1.0);
	FragColor = Color;
}

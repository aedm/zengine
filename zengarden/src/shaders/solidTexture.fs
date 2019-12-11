:name "SolidTexture.fs"

:param sampler2D uTexture
:input vec2 vTexCoord
:output vec4 FragColor;

SHADER
{
	FragColor = texture2D(uTexture, vTexCoord);
}

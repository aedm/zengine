:name "SolidTexture.fs"

:param sampler2D uTexture
:input vec2 vTexCoord

SHADER
{
	gl_FragColor = texture2D(uTexture, vTexCoord);
}

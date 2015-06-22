:name "TextTexture"

:param sampler2D uTexture
:param vec4 uColor

:input vec2 vTexCoord

SHADER
{
	vec4 c = texture2D(uTexture, vTexCoord);
	gl_FragColor = vec4(uColor.rgb, uColor.a * c.a);
}

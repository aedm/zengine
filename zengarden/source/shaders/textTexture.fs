uniform sampler2D uTexture;
uniform vec4 uColor;

varying vec2 vTexCoord;

void main()
{
	vec4 c = texture2D(uTexture, vTexCoord);
	gl_FragColor = vec4(uColor.rgb, uColor.a * c.a);
}

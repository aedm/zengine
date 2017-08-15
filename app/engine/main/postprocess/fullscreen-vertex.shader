:name "PostProcess vertex shader for fullscreen quads"

:input vec3 aPosition;
:input vec2 aTexCoord;

:output vec2 vTexCoord
:output vec3 vNormal

SHADER
{
	gl_Position = vec4(aPosition, 1.0); 
	vTexCoord = aTexCoord;
}
:name "Default VS"
:returns void

:input vec3 aPosition;
:input vec3 aNormal;
:input vec2 aTexCoord;

:global mat4 gTransformation;

:output vec3 vNormal
:output vec2 vTexCoord

SHADER
{
	gl_Position = vec4(aPosition, 1.0) * gTransformation; 
	vNormal = aNormal;
	vTexCoord = aTexCoord;
}
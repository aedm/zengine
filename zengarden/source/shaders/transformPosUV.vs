:name "TransformPosUv.vs"

:input vec3 aPosition
:input vec2 aTexCoord

:global mat4 gTransformation

:output vec2 vTexCoord

SHADER
{
	gl_Position = vec4(aPosition, 1) * gTransformation;
	vTexCoord = aTexCoord;
}

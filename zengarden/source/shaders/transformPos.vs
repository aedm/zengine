:name "TransformPos.vs"

:input vec3 aPosition

:global mat4 gTransformation

SHADER
{
	gl_Position =  vec4(aPosition, 1) * gTransformation;
}

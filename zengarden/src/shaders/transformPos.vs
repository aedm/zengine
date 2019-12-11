:name "TransformPos.vs"

:input vec3 aPosition

:global mat4 gTransformation

SHADER
{
	gl_Position = gTransformation * vec4(aPosition, 1);
}

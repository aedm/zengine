:name "testvs"
:returns void

:input vec3 aPosition;
:global mat4 gTransformation;

SHADER
{
	gl_Position = vec4(aPosition, 0.1) * gTransformation; 
}
